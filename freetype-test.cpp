#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <iostream>

#define FONT_SIZE 36
//This file exists to check that programs that use freetype / harfbuzz link properly in this base code.
//You probably shouldn't be looking here to learn to use either library.

// do as much of the tasks as you can in this one file in order
// figure out how to restructure in a nice way in your own code

// code inspired by:
// https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c

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
	const char* font_path = "/Users/arililoia/CarnegieMellonCS/game-programming/my-game4/Knewave-Regular.ttf";
	if (FT_New_Face(library, font_path, 0, &face) != 0){
		std::cerr << "fucked up face " << std::endl;
		return 1;
	}

	// hb docs - You must set the face size on ft_face before calling hb_ft_font_create() on it.
	// is that what FT_Set_Char_Size does?
	if (FT_Set_Char_Size(face, FONT_SIZE*64, FONT_SIZE*64, 0, 0) != 0){
		std::cerr << "fucked up char size " << std::endl;
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
	hb_glyph_info_t *info = hb_buffer_get_glyph_infos(hb_buffer, &info_len);
	std::cout << "info_len: " << info_len << std::endl;
	// hb_glyph_position_t(x_advance, y_advance, x_offset, y_offset)
	// advances are how much the line advances after drawing the glyph
	// offsets are how the glyph moves on teh x-axis befoer drawing it
	unsigned int positions_len;
	hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(hb_buffer, &positions_len);
	std::cout << "positions_len: " << positions_len << std::endl;

	// only here so that i don't get compiler errors when these values not used
	if((info == nullptr) || (pos == nullptr)){
		std::cout << std::endl;
	}
	
	std::cout << "pos[1].x_advance: " << pos[1].x_advance << std::endl;
	// IDEA - working with line breaks.. instead of creating a new buffer every time,
	// we can edit this value?

	// at this point: have successfully converted text and a font to glyphs and positions
	// the following code usees this documentation:
	// https://freetype.org/freetype2/docs/tutorial/step1.html


	// iterate over info to get glyph indices
	// pass glyph index into FT_Load_Glyph
	// use FT_Render_Glyph to convert to bitmap
	// Once you have a bitmapped glyph image, you can access it directly through glyph->bitmap 
	FT_UInt glyph_index;
	for (int i=0; i<info_len; i++){
		// load glyph into face glyph slod 
		glyph_index =  FT_Get_Char_Index(face, text[i]);
		if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT) != 0){
			std::cout << "glyph loading failed" << std::endl;
		}
		// render glyph as bitmap
		if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL) != 0){
			std::cout << "glyph rendering failed" << std::endl;
		}
	}


	// face objects contain 1+ tables / charmaps to convert character codes to glyph indices
	// face objects contain only one glyph slot




	

	// in class:
	FT_Done_FreeType(library);
	// so at this point we have an intiialized library?

	std::cout << "It worked?" << std::endl;
}
