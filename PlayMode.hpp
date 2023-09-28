#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

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

// struct Choice {
// 	int dest_page;
// 	std::string option;
// };

struct Page{
	int page_number;
	std::string page_text;
	int dest_page_1;
	int dest_page_2;
};

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
	void load_story();
	void zero_out_image_buffer();
	int load_page2display(int page_number);
	// int load_choices2display(int page_number, int choice_number);
	int load_full_page(int page_number);
	int current_page = 0;
	bool change_tex = true;
	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	// Scene scene;
	std::vector<Page> Story;

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > leg_tip_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;

	FT_Library library;
	FT_Face face;

};

