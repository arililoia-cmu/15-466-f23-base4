#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"
#include "TextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

// #include <ft2build.h>
// #include FT_FREETYPE_H

// #include <hb.h>
// #include <hb-ft.h>

// #include <iostream>
// #include <iterator>
// #include <fstream>
// #include <sstream>
// #include <vector>
// #include <string>

#define FONT_SIZE  50
#define FONT_SCALE 64
#define CHAR_RESOLUTION 64
#define WIDTH   1280
#define HEIGHT  720
#define MARGIN 30
#define PEN_X_START (MARGIN * CHAR_RESOLUTION)
#define PEN_Y_START (HEIGHT - (FONT_SIZE + MARGIN)) * CHAR_RESOLUTION
#define PAGE_TEXT_END 200
unsigned char image[HEIGHT][WIDTH];


Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("bugsim_music.wav"));
});

void PlayMode::load_story(){
	// import choices
	// std::cout << "load story" << std::endl;
	std::ifstream file(data_path("choices.csv"));
	if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
    }

	std::string choice_row;
	std::string choice_element;
	int acc = 0;
	// std::vector<Page> Story;
	while(std::getline(file, choice_row, '\n')) {
		// would be better to allocate everything in memory first given
		// the number of csv rows
		// std::cout << "get line of story" << std::endl;
		Page one_page;
		// std::cout << "getting line " << std::endl;
		// csv reading code inspired by:
		// https://stackoverflow.com/questions/275355/c-reading-file-tokens#275405
		std::istringstream stream(choice_row);
		while (std::getline(stream, choice_element, ',')) {
			// std::cout << acc << std::endl;
			// std::cout << "choice_elementL: " << choice_element << std::endl;
			if (acc == 0){
				one_page.page_number = stoi(choice_element);
			}
			else if (acc == 1){
				one_page.page_text = choice_element;
			}
			else if (acc == 2){
				one_page.dest_page_1 = stoi(choice_element);
			}
			else if (acc == 3){
				one_page.dest_page_2 = stoi(choice_element);
			}
			acc++;
		}
		Story.push_back(one_page);
		acc = 0;
	
	}


	for (int i=0; i<(Story.size()); i++){
		std::cout << Story.at(i).page_number << " " <<  Story.at(i).dest_page_1 << " " <<  Story.at(i).dest_page_2 << std::endl;
	}

	file.close();
}


// draw_bitmap code taken from the  linked `example1.c` file from this tutorial:
// https://freetype.org/freetype2/docs/tutorial/step1.html
void draw_bitmap( FT_Bitmap*  bitmap, FT_Int x, FT_Int y){
 
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;

  for ( i = x, p = 0; i < x_max; i++, p++ ){
    for ( j = y, q = 0; j < y_max; j++, q++ ){
      if ( i < 0 || j < 0 || i >= WIDTH || j >= HEIGHT ){
		continue;
	  }
      image[j][i] |= bitmap->buffer[q * bitmap->width + p];
    }
  }
}

void PlayMode::zero_out_image_buffer(){
	// std::cout << "zoob" << std::endl;
	for (int i=0; i<HEIGHT; i++){
		for (int j=0; j<WIDTH; j++){
			image[i][j] = 0;
		}
	}
}

int PlayMode::load_full_page(int page_number){
	zero_out_image_buffer();
	// std::cout << "loadfullpage" << std::endl;
	if (load_page2display(page_number) != 0){
		return 1;
	}
	current_page = page_number;
	return 0;
}


// load the page we want to display into an image
int PlayMode::load_page2display(int page_number){
	// std::cout << "load_page2display" << std::endl;


	// code based on freetype tutorial begins here:
	// https://freetype.org/freetype2/docs/tutorial/step1.html
	if (FT_Init_FreeType( &library ) != 0){
		std::cerr << "fucked up initailzating freetype " << std::endl;
		return 1;
	}

	// Roboto Regular font designed by Christian Robertson taken from here:
	// https://fonts.google.com/specimen/Roboto
	if (FT_New_Face(library, data_path("Roboto-Regular.ttf").c_str(), 0, &face) != 0){
		std::cerr << "bad face " << std::endl;
		return 1;
	}

	if (FT_Set_Char_Size(face, FONT_SIZE * FONT_SCALE, FONT_SIZE * FONT_SCALE, CHAR_RESOLUTION, 0) != 0){
		std::cerr << "bad char size " << std::endl;
		return 1;
	}

	/// code based on harfbuzz tutorial begins here:
	// https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
	hb_font_t *hb_font;
	hb_font = hb_ft_font_create_referenced(face);
	hb_buffer_t *hb_buffer;
	hb_buffer = hb_buffer_create();

	// char conversion idea from this stackexchange post:
	// https://stackoverflow.com/questions/347949/how-to-convert-a-stdstring-to-const-char-or-char
	const char *text = Story.at(page_number).page_text.c_str();
	// std::cout << text << std::endl;
	hb_buffer_add_utf8(hb_buffer, text, -1, 0, -1);
	hb_buffer_guess_segment_properties(hb_buffer);
	hb_shape(hb_font, hb_buffer, NULL, 0);


	// unsigned int buffer_len = hb_buffer_get_length (hb_buffer);
	// std::cout << "buffer_len: " << buffer_len << std::endl;
	unsigned int info_len;
	hb_glyph_info_t *g_info = hb_buffer_get_glyph_infos(hb_buffer, &info_len);
	// std::cout << "info_len: " << info_len << std::endl;
	// unsigned int positions_len;
	// hb_glyph_position_t *g_pos = hb_buffer_get_glyph_positions(hb_buffer, &positions_len);

	FT_GlyphSlot slot = face->glyph;

	FT_Vector     pen;   
	pen.x = PEN_X_START;
    pen.y = PEN_Y_START;
	int hacky_line_len = 0;
	// std::cout << "info_len:  " << info_len << std::endl;
	for (int i=0; i<info_len; i++){
		// std::cout << "i: "  << i << std::endl; 
		FT_Set_Transform( face, NULL, &pen );

		/* load glyph image into the slot (erase previous one) */
		int error = FT_Load_Glyph(face, (FT_ULong)(g_info[i].codepoint), FT_LOAD_RENDER );
		if ( error )
			continue;  /* ignore errors */

		int target_height = HEIGHT;
		

		// std::cout << "slot->bitmap_left: " << slot->bitmap_left << std::endl;
		// std::cout << "slot->bitmap_top: " << slot->bitmap_top << std::endl;
		// std::cout << "slot->advance.x: " << slot->advance.x/64. << std::endl;

		draw_bitmap( &slot->bitmap,
                 slot->bitmap_left,
                 target_height - slot->bitmap_top );

		/* increment pen position */
		hacky_line_len += slot->advance.x/64. + 2;

		// if ((pen.x + slot->advance.x + MARGIN + 10) >= WIDTH){
		if ((hacky_line_len + MARGIN) > WIDTH){
			pen.x = PEN_X_START;
			pen.y -= FONT_SIZE * CHAR_RESOLUTION;
			// std::cout << "pen.y: " << pen.y << std::endl;
			hacky_line_len = 0;
		}else{
			pen.x += slot->advance.x;
			pen.y += slot->advance.y;
		}

		if (pen.y <= PAGE_TEXT_END * CHAR_RESOLUTION){
			std::cout << "pen.y <= PAGE_TEXT_END * CHAR_RESOLUTION: " << std::endl;
			FT_Done_FreeType(library);
			return 0;
		}

	
	}

	FT_Done_FreeType(library);
	// code based on freetype tutorial linked above ends here:

	return 0;

}


PlayMode::PlayMode()  {

	//start music loop playing:
	// (note: position will be over-ridden in update())
	leg_tip_loop = Sound::loop(*dusty_floor_sample, 1.0f, 0.0f);

	load_story();
	if (load_full_page(current_page) != 0){
		std::cout << "no" << std::endl;
	}

}


PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_1){
			// std::cout << "KEYdown 1" << std::endl;
			left.downs += 1;
			left.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_2){
			// std::cout << "KEYdown 2" << std::endl;
			right.downs += 1;
			right.pressed = true;
			return true;
		}

	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_1) {
			std::cout << "KEYUP 1" << std::endl;
			left.pressed = false;
			change_tex = true;
			std::cout << "from page " << current_page;
			int get_opt_1 = Story.at(current_page).dest_page_1;
			load_full_page(get_opt_1);
			std::cout << " loading page " << get_opt_1 << std::endl;
			
			return true;
		} else if (evt.key.keysym.sym == SDLK_2) {
			std::cout << "KEYUP 2" << std::endl;
			right.pressed = false;
			change_tex = true;
			int get_opt_2 = Story.at(current_page).dest_page_2;
			load_full_page(get_opt_2);
			std::cout << "loading page " << get_opt_2 << std::endl;
			return true;
		} 

	} 

	return false;
}

void PlayMode::update(float elapsed) {


	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {

	// code developed in office hours with Prof. Jim McCann starts here
	//----------------------------------------------
	//Test code: draw a textured quad!
	//based -- in part -- on code from LitColorTextureProgram

	// generate huge tex_data
	std::vector< glm::u8vec4 > tex_data;
	for (int row=HEIGHT; row>0; row--){
		for (int col=0; col<WIDTH; col++){
			if (image[row][col] == 0){
				tex_data.push_back(glm::u8vec4(0x00, 0x00, 0x00, 0xff));
			}else{
				tex_data.push_back(glm::u8vec4(0xff, 0xff, 0xff, 0xff));
			}
		}
	}




	static GLuint tex = 0;
	if (tex == 0 || change_tex == true) {

		glGenTextures(1, &tex);

		glBindTexture(GL_TEXTURE_2D, tex);


		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.data());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	static GLuint buffer = 0;
	if (buffer == 0 || change_tex == true) {
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		//actually nothing to do right now just wanted to bind it for illustrative purposes
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	struct Vert {
		Vert(glm::vec3 const &position_, glm::vec2 const &tex_coord_) : position(position_), tex_coord(tex_coord_) { }
		glm::vec3 position;
		glm::vec2 tex_coord;
	};
	static_assert(sizeof(Vert) == 20, "Vert is packed");

	auto &program = texture_program;

	static GLuint vao = 0;
	if (vao == 0 || change_tex == true ) {
		//based on PPU466.cpp

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, buffer);

		glVertexAttribPointer(
			program->Position_vec4, //attribute
			3, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vert), //stride
			(GLbyte *)0 + offsetof(Vert, position) //offset
		);
		glEnableVertexAttribArray(program->Position_vec4);

		glVertexAttribPointer(
			program->TexCoord_vec2, //attribute
			2, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vert), //stride
			(GLbyte *)0 + offsetof(Vert, tex_coord) //offset
		);
		glEnableVertexAttribArray(program->TexCoord_vec2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);
	}

	if (change_tex == true){
		change_tex = false;
	}


	//actually draw some textured quads!
	std::vector< Vert > attribs;

	attribs.emplace_back(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
	attribs.emplace_back(glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec2(0.0f, 1.0f));
	attribs.emplace_back(glm::vec3( 1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
	attribs.emplace_back(glm::vec3( 1.0f,  1.0f, 0.0f), glm::vec2(1.0f, 1.0f));


	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vert) * attribs.size(), attribs.data(), GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	//as per Scene::draw -
	glUseProgram(program->program);
	glUniformMatrix4fv(program->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

	glBindTexture(GL_TEXTURE_2D, tex);


	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(vao);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, attribs.size());

	glBindVertexArray(0);


	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);

	GL_ERRORS();
	// code developed in office hours with Prof. Jim McCann ends here

}

