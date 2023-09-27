

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#define FONT_SIZE  25
#define FONT_SCALE 64
// char resolution
#define CHAR_RESOLUTION 38
#define MAX_QUESTION_BITMAP_LENGTH 100
#define MAX_QUESTION_BITMAP_HEIGHT 50


#define WIDTH   100
#define HEIGHT  57
#define PEN_X_START 59 * 32
#define PEN_Y_START 50 * 32



/* origin is the upper left corner */
unsigned char image[HEIGHT][WIDTH];


// ^ this is the smallest char resolution can be without an error getting thrown

//This file exists to check that programs that use freetype / harfbuzz link properly in this base code.
//You probably shouldn't be looking here to learn to use either library.

// do as much of the tasks as you can in this one file in order
// figure out how to restructure in a nice way in your own code

// code inspired by:
// https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c

void draw_bitmap( FT_Bitmap*  bitmap, FT_Int x, FT_Int y){
 
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;


  /* for simplicity, we assume that `bitmap->pixel_mode' */
  /* is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)   */
  std::cout << "x, xmax =  " << x << " " << x_max << std::endl;
  std::cout << "y, ymax =  " << y << " " << y_max << std::endl;

  for ( i = x, p = 0; i < x_max; i++, p++ ){
    for ( j = y, q = 0; j < y_max; j++, q++ ){
      if ( i < 0 || j < 0 || i >= WIDTH || j >= HEIGHT ){
		continue;
	  }
      image[j][i] |= bitmap->buffer[q * bitmap->width + p];
    }
  }
}

void show_image( void ){
  int  i, j;
  for ( i = 0; i < HEIGHT; i++ ){
    for ( j = 0; j < WIDTH; j++ ){
		putchar( image[i][j] == 0 ? ' ' : image[i][j] < 128 ? '+' : '*' );
	}
    putchar( '\n' );
  }
}

struct Choice {
	int dest_page;
	std::string option;
};

struct Page{
	int page_number;
	std::string page_text;
	std::vector<Choice> page_choices;
};


int main(int argc, char **argv) {


	std::ifstream file("choices.csv");
	if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }

	// 
	std::string choice_row;
	std::string choice_element;
	int acc = 0;

	std::vector<Page> Story;
	

	while(std::getline(file, choice_row, '\n')) {
		
		// would be better to allocate everything in memory first given
		// the number of csv rows

		Page one_page;
		std::vector<Choice> page_choices;
		Choice one_choice;

		// csv reading code inspired by:
		// https://stackoverflow.com/questions/275355/c-reading-file-tokens#275405
		std::istringstream stream(choice_row);
		while (std::getline(stream, choice_element, ',')) {
			// tokens.push_back(choice_element);
			// std::cout << choice_element << std::endl;
			if (acc == 0){
				one_page.page_number = stoi(choice_element);
			}
			else if (acc == 1){
				one_page.page_text = choice_element;
			}
			else{
				if (acc%2 == 0){
					one_choice.dest_page = stoi(choice_element);
				}
				else if (acc%2 == 1){
					one_choice.option = choice_element;
					page_choices.push_back(one_choice);
				}
			}
			acc++;
		}
		one_page.page_choices = page_choices;
		Story.push_back(one_page);
		acc = 0;
	
	}
	// std::cout << "Story[0].page_number: " << Story[0].page_number  << std::endl;
	// std::cout << "Story[0].page_text: " << Story[0].page_text  << std::endl;
	// std::cout << "Story[0].page_choices.size(): " << Story[0].page_choices.size()  << std::endl;
	// for (int i=0; i<Story[0].page_choices.size(); i++){
	// 	std::cout << Story[0].page_choices[i].dest_page << std::endl;
	// }
	
	file.close();

	




	std::cout << " " << std::endl;
	std::cout << " " << std::endl;
	

	FT_Library library;
	if (FT_Init_FreeType( &library ) != 0){
		std::cerr << "fucked up initailzating freetype " << std::endl;
		return 1;
	}

	// want to use new face (open face too hard)
	FT_Face face;
	const char* font_path = "/Users/arililoia/CarnegieMellonCS/game-programming/my-game4/Roboto-Regular.ttf";
	if (FT_New_Face(library, font_path, 0, &face) != 0){
		std::cerr << "bad face " << std::endl;
		return 1;
	}

	// hb docs - You must set the face size on ft_face before calling hb_ft_font_create() on it.
	// is that what FT_Set_Char_Size does?
	// FT_Char_Size(face, char_width, char_height, horz-resoution, vert-resoltion)
	// setting the width/resolution in 1 dimension to 0 sets it equal to the other dimension

	if (FT_Set_Char_Size(face, FONT_SIZE * FONT_SCALE, FONT_SIZE * FONT_SCALE, CHAR_RESOLUTION, 0) != 0){
		std::cerr << "bad char size " << std::endl;
		return 1;
	}

	// std::cout << "face->size: " << &(face->size) << std::endl;
	// hb_font_t - data type for holding fonts
	hb_font_t *hb_font;
	// hb_ft_font_create () - Creates an hb_font_t font object from the specified FT_Face.
	// docs said use hb_ft_font_create_referenced unless we have a good reason not to
	hb_font = hb_ft_font_create_referenced(face);
	
	
	// hb_buffer_t - input and output buffers
	hb_buffer_t *hb_buffer;
	hb_buffer = hb_buffer_create();
	// before shaping - buffers hold input characters passed to hb_shape()
	// input buffer is a sequence of Unicode codepoints, with associated attributes such as direction and script
	// after shaping  - the hold output glyphs.
	// output buffer is a sequence of glyphs, with associated attributes such as position and cluster.
	const char *text = "tej";
	// hb_buffer_add_utf8(buffer, text, text length, item offset, item length)
	// replaces invalid utf-8 chars with the buffer replacement codepoint
	hb_buffer_add_utf8(hb_buffer, text, -1, 0, -1);
	// hb_buffer_guess_segment_properties
	// sets buffer segment properties based on buffer unicode contents
	// fixes issues we don't know about
	hb_buffer_guess_segment_properties(hb_buffer);

	// shaping
	// hb_shape(*font, *buffer, *features, num_features)
	hb_shape(hb_font, hb_buffer, NULL, 0);
	// the buffer itself gets reshaped in place
	

	// get glyph info and positions out of the buffer
	unsigned int buffer_len = hb_buffer_get_length (hb_buffer);
	std::cout << "buffer_len: " << buffer_len << std::endl;
	// but: hb_glyph_info_t is the structure that holds information about the glyphs and their relation to input text.
	// hb_codepoint_t hb_glyph_info_t->codepoint: either a unicode code point (before shapng) or a glyph index (after shaping)
	// uint32_t hb_glyph_info_t->cluster: the index of the char in the OG text corresponding to this object
	
	unsigned int info_len;
	hb_glyph_info_t *g_info = hb_buffer_get_glyph_infos(hb_buffer, &info_len);
	std::cout << "info_len: " << info_len << std::endl;
	// hb_glyph_position_t(x_advance, y_advance, x_offset, y_offset)
	// advances are how much the line advances after drawing the glyph
	// offsets are how the glyph moves on teh x-axis befoer drawing it
	unsigned int positions_len;
	hb_glyph_position_t *g_pos = hb_buffer_get_glyph_positions(hb_buffer, &positions_len);
	std::cout << "positions_len: " << positions_len << std::endl;

	// only here so that i don't get compiler errors when these values not used
	// if((g_info == nullptr) || (g_pos == nullptr)){
	// 	std::cout << std::endl;
	// }
	printf("Raw buffer contents:\n");
	for (unsigned int i = 0; i < buffer_len; i++)
	{
		hb_codepoint_t gid   = g_info[i].codepoint;
		unsigned int cluster = g_info[i].cluster;
		double x_advance = g_pos[i].x_advance / 64.;
		double y_advance = g_pos[i].y_advance / 64.;
		double x_offset  = g_pos[i].x_offset / 64.;
		double y_offset  = g_pos[i].y_offset / 64.;

		char glyphname[32];
		hb_font_get_glyph_name (hb_font, gid, glyphname, sizeof (glyphname));

		printf("glyph='%s'	cluster=%d	advance=(%g,%g)	offset=(%g,%g)\n",
				glyphname, cluster, x_advance, y_advance, x_offset, y_offset);
	}

	printf ("Converted to absolute positions:\n");





	
	for (int i=0; i<positions_len; i++){
		std::cout << "pos[" << i << "].x_advance: " << g_pos[i].x_advance/64. << std::endl;
		std::cout << "pos[" << i << "].y_advance: " << g_pos[i].y_advance << std::endl;
		std::cout << "pos[" << i << "].x_offset: " << g_pos[i].x_offset << std::endl;
		std::cout << "pos[" << i << "].y_offset: " << g_pos[i].y_offset << std::endl;
	// std::cout << "pos[0].x_advance: " << g_pos[0].y_advance << std::endl;

	}
	// std::cout << "pos[0].x_advance: " << g_pos[0].x_advance << std::endl;
	// std::cout << "pos[0].x_advance: " << g_pos[0].y_advance << std::endl;
	
	// IDEA - working with line breaks.. instead of creating a new buffer every time,
	// we can edit this value?

	// at this point: have successfully converted text and a font to glyphs and positions
	// the following code usees this documentation:
	// https://freetype.org/freetype2/docs/tutorial/step1.html


	FT_GlyphSlot slot = face->glyph;

	FT_Vector     pen;   
	pen.x = PEN_X_START;
    pen.y = PEN_Y_START;

	for (int i=0; i<info_len; i++){
		// load glyph into face glyph slot
		// could use FT_load_char instead
		// don't have to call everything explicitly
		// but will do so for debugging


		/* retrieve glyph index from character code */
		// glyph_index = FT_Get_Char_Index(face, text[i]);
		// FT_UInt glyph_index = (FT_UInt)(g_info[i].codepoint);
		FT_Set_Transform( face, NULL, &pen );

		/* load glyph image into the slot (erase previous one) */
		int error = FT_Load_Glyph(face, (FT_ULong)(g_info[i].codepoint), FT_LOAD_RENDER );
		if ( error )
			continue;  /* ignore errors */

		int target_height = HEIGHT;
		
		draw_bitmap( &slot->bitmap,
                 slot->bitmap_left,
                 target_height - slot->bitmap_top );

		/* increment pen position */
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;

	}
	show_image();


	// in class:
	FT_Done_FreeType(library);
	// so at this point we have an intiialized library?

	std::cout << "It worked?" << std::endl;
}
