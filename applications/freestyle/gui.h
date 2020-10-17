#pragma once
#include <cppgl/named_map.h>
#include <imgui/imgui.h>
#include <unordered_map>
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include "rendering.h"

struct colorscheme {
	ImVec4 transparency;
	ImVec4 background_window;
	ImVec4 background_window_inactive;
	ImVec4 border_color;
	ImVec4 widget_color;
	ImVec4 widget_hovered_color;
	ImVec4 widget_active_color;
	ImVec4 text_color;
	ImVec4 loading_bar_color;
	ImVec4 loading_bar_background_color;
};

namespace GUI {

	enum FONT {
		NOT_SPECIFIED,
		PLAYER_STATS_FONT,
		RETRO,
		ARROWS
	};

	class container_t {
		public:
			std::string id;
			bool active;
			glm::vec2 pos;
			glm::vec2 dim;
			glm::vec4 color;
			float rounding;
			FONT font;
			uint font_size_optional = 0;
			glm::vec4 border_col;
			float border_size;
			bool isChild;
			std::string text;

			container_t(std::string id_p, bool active_p, glm::vec2 pos_p, glm::vec2 dim_p, glm::vec4 color_p, glm::vec4 border_col_p, float border_size_p, float rounding_p, FONT font_p, bool isChild_p, std::string text_p) {
				id = id_p;
				active = active_p;
				pos = pos_p;
				dim = dim_p;
				color = color_p;
				rounding = rounding_p;
				font = font_p;
				border_col = border_col_p;
				border_size = border_size_p;
				isChild = isChild_p;
				text = text_p;
			}
			virtual void draw() = 0;
			virtual bool isClicked() = 0;
			virtual std::string getClicked() = 0;
			virtual bool get_pressed_enter() = 0;
	};

	class overlay_object {
		public:
		glm::vec2 pos;
		std::string text;
		bool active;
		std::string type;
		bool center_aligned;

		overlay_object(glm::vec2 pos_p, std::string text_p, bool active_p, std::string type_p, bool center_aligned_p) {
			pos = pos_p;
			text = text_p;
			active = active_p;
			type = type_p;
			center_aligned = center_aligned_p;
		}
	};

	struct pixel_padding {
		glm::ivec2 front_pad;
		glm::ivec2 back_pad;
	};

	void init_GUI();

	void set_bar_char_names(std::vector<std::string> new_names);

	ImFont* get_font(FONT f, uint size);

	glm::vec2 pixels_to_percent(glm::ivec2 v, glm::ivec2 resolution);
	// glm::vec2 pixels_to_percent(glm::vec2 v, glm::ivec2 resolution);

	uint create_window(glm::vec2 pos, glm::vec2 dim, glm::vec4 color, glm::vec4 border_col, float border_size, int window_flags, float rounding = 0.f, pixel_padding pads = pixel_padding { glm::ivec2(0,0), glm::ivec2(0,0)}, FONT font = NOT_SPECIFIED, bool isChild = false, ImGuiWindowFlags_ remove_flags = ImGuiWindowFlags_None);

	uint create_collapse_window(glm::vec2 pos, glm::vec2 dim, glm::vec4 color, glm::vec4 border_col, float border_size, int window_flags, float rounding, pixel_padding pads, FONT font_p, bool isChild);

	uint create_button(std::string text, glm::vec2 pos, glm::vec2 dim, glm::vec4 color, glm::vec4 color_hovered, glm::vec4 color_active, glm::vec4 border_col, float border_size, float rounding, FONT font_p, std::function<void()> hovered_callback_p = 0);

	bool isClicked(uint container_id);

	void draw();
	void draw(std::vector<uint> ids);
	void set_percentage(int index, float percentage);
	void reset_percentage();
	void set_char_name(int index, std::string char_name);
	void draw_loading_screen(float percent, std::string text, ImVec4 color = ImVec4(-1,-1,-1,-1), ImVec4 color_bg = ImVec4(-1,-1,-1,-1));
}

class Screen {
	public:
	// data
	static glm::vec2 start_screen_right_pos;
	static glm::vec2 start_screen_right_dim;
	std::string name;
	std::string inserted_name;
	std::vector<std::string> texts_input;
	uint scroll_page = 0;
	int dec_page_button = -1;
	int inc_page_button = -1;
	uint files_text_child_id;
	uint overlay_hint_id = -1;
	std::vector<uint> load_game_collapse_window_ids = {};
	std::vector<uint> load_files_collapse_window_ids = {};
	std::vector<uint> how_to_play_window_ids = {};
	std::vector<uint> insert_name_window_ids = {};
	std::shared_ptr<std::vector<uint>> screen_objects;
	std::shared_ptr<std::vector<uint>> Buttons;
	std::shared_ptr<Texture2D> background_image = 0;
	static Screen* current();
	void make_current();
	static std::unordered_map<uint, std::shared_ptr<GUI::container_t>> containers;

	// functions
	Screen(std::string name);

	void generate_overlay();
	void generate_victory_screen();
	void generate_start_screen();
	void generate_fight_screen();
	void generate_text_screen();
	void generate_text_and_okay_screen(bool okay = true);
	void generate_health_bar_screen();
	void generate_checkpoint_screen();
	void update_checkpoint_screen();
	void draw();
	bool draw_healthbar_anim(bool start, int player);
	void draw_victory_screen();
	void draw_checkpoint_screen();
	void draw_start_screen();
	void update_overlay();
	void set_overlay_hint(std::string new_string);
	void set_scroll_page(uint page);
	void use_texts_input(std::vector<std::string> texts_p, bool only_buttons = false);
	std::string get_clicked_text_id();
	void set_health_percentage(int bar_index, float percent);
};
