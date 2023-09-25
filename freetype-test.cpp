#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <iostream>

#define FONT_SIZE 36
#define CHAR_DIM  50 * 64
// char resolution
#define CHAR_RESOLUTION 100
// ^ this is the smallest char resolution can be without an error getting thrown
#define WIDTH  50 * 64 * 4
#define HEIGHT  50 * 64
//This file exists to check that programs that use freetype / harfbuzz link properly in this base code.
//You probably shouldn't be looking here to learn to use either library.

// do as much of the tasks as you can in this one file in order
// figure out how to restructure in a nice way in your own code

// code inspired by:
// https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
unsigned char image[HEIGHT][WIDTH];



// draw_bitmap taken from here:
// https://freetype.org/freetype2/docs/tutorial/example1.c
void draw_bitmap( FT_Bitmap*  bitmap, FT_Int  x, FT_Int y) {
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;
  /* for simplicity, we assume that `bitmap->pixel_mode' */
  /* is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)   */
  for ( i = x, p = 0; i < x_max; i++, p++ ){
    for ( j = y, q = 0; j < y_max; j++, q++ ){
      if ( i < 0 || j < 0  || i >= WIDTH || j >= HEIGHT )
        continue;
      image[j][i] |= bitmap->buffer[q * bitmap->width + p];
    }
  }
}

void show_image( void ){
  int  i, j;
  for ( i = 0; i < HEIGHT; i++ ){
    for ( j = 0; j < WIDTH; j++ ){
		putchar( image[i][j] == 0 ? ' ' : image[i][j] < 128 ? '+'  : '*' ); 
	}
	putchar( '\n' );
  }
}

int main(int argc, char **argv) {

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

	if (FT_Set_Char_Size(face, CHAR_DIM, 0, CHAR_RESOLUTION, 0) != 0){
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
	const char *text = "test";
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
	if((g_info == nullptr) || (g_pos == nullptr)){
		std::cout << std::endl;
	}
	
	std::cout << "pos[1].x_advance: " << g_pos[1].x_advance << std::endl;
	// IDEA - working with line breaks.. instead of creating a new buffer every time,
	// we can edit this value?

	// at this point: have successfully converted text and a font to glyphs and positions
	// the following code usees this documentation:
	// https://freetype.org/freetype2/docs/tutorial/step1.html


	FT_GlyphSlot slot = face->glyph;
	// FT_UInt glyph_index;
	// int pen_x = 0;
	// int pen_y = 0;

	for (int i=0; i<info_len; i++){
		// load glyph into face glyph slot
		// could use FT_load_char instead
		// don't have to call everything explicitly
		// but will do so for debugging


		// why did this not work and that did work?
		/*
		glyph_index =  FT_Get_Char_Index(face, text[i]);
		if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT) != 0){
			std::cout << "glyph loading failed" << std::endl;
		}
		// render glyph as bitmap
		if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO) != 0){
			std::cout << "glyph rendering failed" << std::endl;
		}
		*/
		if (FT_Load_Char( face, text[i], FT_LOAD_RENDER ) != 0){
			continue;
		}


		std::cout << i << std::endl;
		std::cout << "slot->bitmap.width: " << slot->bitmap.width << std::endl;
		std::cout << "slot->bitmap.rows: " << slot->bitmap.rows << std::endl;
		// for (int j=0; j<(slot->bitmap.width*slot->bitmap.rows); j++){
		// 	// putchar(slot->bitmap.buffer[j]);
		// 	// putchar( slot->bitmap.buffer[j] == 0 ? ' ' : image[i][j] < 128 ? '+'  : '*' ); 
		// 	putchar(slot->bitmap.buffer[j] == 0 ? ' ' : '*'); 
		// }
		int acc = 0;
		for (int j=0; j<slot->bitmap.rows; j++){
			for (int m=0; m<slot->bitmap.width; m++){
				putchar(slot->bitmap.buffer[acc] == 0 ? ' ' : '*'); 
				acc++;
			}
			std::cout << std::endl;
		}


		// draw_bitmap( &slot->bitmap,
        // //           pen_x + slot->bitmap_left,
        // //           pen_y - slot->bitmap_top );
		// pen_x += slot->advance.x >> 6;
		// pen_y += slot->advance.y >> 6;


	}



	// iterate over info to get glyph indices
	// pass glyph index into FT_Load_Glyph
	// use FT_Render_Glyph to convert to bitmap
	// Once you have a bitmapped glyph image, you can access it directly through glyph->bitmap 
	// FT_UInt glyph_index;
	// FT_GlyphSlot slot = face->glyph;
	// FT_Vector pen;
	// pen.x = 300;
	// pen.y = 200;
	// int my_target_height = 10;
	// for (int i=0; i<info_len; i++){
		
	// 	// slot->glyph_index = 
	// 	// transform the thing based on the pen
	// 	// we use null for the matrix as we're just translating
	// 	FT_Set_Transform(face, NULL, &pen);
		
	// 	// load glyph into face glyph slot
	// 	glyph_index =  FT_Get_Char_Index(face, text[i]);
	// 	if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT) != 0){
	// 		std::cout << "glyph loading failed" << std::endl;
	// 	}
	// 	// render glyph as bitmap
	// 	if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL) != 0){
	// 		std::cout << "glyph rendering failed" << std::endl;
	// 	}

	// 	draw_bitmap( &slot->bitmap,
    //               slot->bitmap_left,
    //               my_target_height - slot->bitmap_top );

	// 	pen.x += slot->advance.x;
	// 	pen.y += slot->advance.y;
	// }

	// print bitmap
	// show_image();
	// for (int i=0; i<HEIGHT; i++){
	// 	std::cout << i << ": ";
	// 	for (int j=0; j<WIDTH; j++){
	// 		std::cout << (char)image[i][j];
	// 	}
	// 	std::cout << std::endl;
	// }

	



	// for (int i=0; i<info_len; i++){
	// 	// load glyph into face glyph slot
	// 	// could use FT_load_char instead
	// 	// don't have to call everything explicitly
	// 	// but will do so for debugging
	// 	glyph_index =  FT_Get_Char_Index(face, text[i]);
	// 	if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT) != 0){
	// 		std::cout << "glyph loading failed" << std::endl;
	// 	}
	// 	// render glyph as bitmap
	// 	if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL) != 0){
	// 		std::cout << "glyph rendering failed" << std::endl;
	// 	}


	// 	draw_bitmap( &slot->bitmap,
    //               pen_x + slot->bitmap_left,
    //               pen_y - slot->bitmap_top );


	// }




	

	// in class:
	FT_Done_FreeType(library);
	// so at this point we have an intiialized library?

	std::cout << "It worked?" << std::endl;
}
