#include "gui.h"
#include "checkpoints.h"
#include <cppgl/context.h>
#include <vector>
#include <cstdio>
#include <iostream>
#include <cppgl/file_util.h>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext.hpp"
#include <map>
#include "save.h"
#include <cppgl/timer.h>
#include "files.h"
#include "world3dhandler.h"
#include "jukebox.h"

using namespace std;
extern bool won;
extern int background_flags;
extern string game_mode;
extern shared_ptr<save_t> current_save;
extern shared_ptr<Files::file> current_file;
extern shared_ptr<Screen> start_screen;
extern shared_ptr<Shader> victory_shader;
std::unordered_map<uint, std::shared_ptr<GUI::container_t>> Screen::containers;
Timer healthbar_timer;
double healthbar_time = 2;
glm::vec2 Screen::start_screen_right_pos, Screen::start_screen_right_dim;

colorscheme green_black {ImVec4(0,0,0,0), ImVec4(0,0,0,0.8f), ImVec4(0.43, 0.43, 0.43, 0.5), ImVec4(0,1,0,1), ImVec4(0,1,0,0.2f), ImVec4(0,0.8f,0,0.5), ImVec4(0, 1.f, 0, 0.5), ImVec4(0, 1, 0, 1), ImVec4(0,1,0,1), ImVec4(0,0,0,1)};
colorscheme yellow_blue {ImVec4(0,0,0,0), ImVec4(0,0,0.3,0.8f), ImVec4(0.43, 0.43, 0.43, 0.5), ImVec4(1,1,0,1), ImVec4(1,1,0,0.2f), ImVec4(0.8f,0.8f,0,0.5), ImVec4(1, 1.f, 0, 0.5), ImVec4(1, 1, 0, 1), ImVec4(1,1,0,1), ImVec4(0,0,0.4,1)};
colorscheme red_black {ImVec4(0,0,0,0), ImVec4(0,0,0,0.8f), ImVec4(0.43, 0.43, 0.43, 0.5), ImVec4(1,0,0,1), ImVec4(1,0,0,0.2f), ImVec4(0.8f,0.f,0,0.5), ImVec4(1, 0, 0, 0.5), ImVec4(1, 0, 0, 1), ImVec4(1,0,0,1), ImVec4(0,0,0,1)};
colorscheme cs = green_black;

int debugs[5] {-1,-1,-1,-1,-1};
float f[10];

namespace GUI {
	glm::vec2 res;

	std::unordered_map<FONT, unordered_map<uint,ImFont*>> fonts;
	std::unordered_map<FONT, std::string> font_paths {
		{PLAYER_STATS_FONT, "render-data/fonts/TerminusTTF-4.47.0.ttf"},
		{NOT_SPECIFIED, "render-data/fonts/DroidSansMono.ttf"},
		{RETRO, "render-data/fonts/TerminusTTF-4.47.0.ttf"},
		{ARROWS, "render-data/fonts/TerminusTTF-Bold-4.47.0.ttf"}
	};

	ImFont* get_font(FONT f, uint s) {
		return fonts[f][s];
	}

	std::vector<float> bar_percentage;
	std::vector<float> bar_percentage_old;
	std::vector<float> bar_percentage_new;
	vector<string> bar_char_names;

	void set_bar_char_names(vector<string> new_names) {
		bar_char_names = new_names;
	}

	uint container_ids = 0;

	template<typename T>
		class collapse_window_t : public container_t {
			public:
				int window_flags;
				shared_ptr<map<int64_t,T>> entries = 0;
				string clicked = "not_clicked";

				collapse_window_t(string id_p, bool active_p, glm::vec2 pos_p, glm::vec2 dim_p, glm::vec4 color_p, glm::vec4 border_col_p, float border_size_p, float rounding_p, int window_flags_p, pixel_padding pad_p, FONT font_p, bool isChild_p) : container_t(id_p, active_p, pos_p, dim_p, color_p, border_col_p, border_size_p, rounding_p, font_p, isChild_p, "window_no_text") {
					window_flags = window_flags_p & (~ImGuiWindowFlags_NoMouseInputs);
				}

				void draw() {
					if(!active) return;
					clicked = "not_clicked";
					ImGui::SetNextWindowFocus();
					ImGui::SetNextWindowPos(ImVec2(pos.x * res.x, pos.y * res.y));
					ImGui::SetNextWindowSize(ImVec2(dim.x * res.x, dim.y * res.y));
					ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, border_size);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, rounding);
					ImGui::PushStyleColor(ImGuiCol_WindowBg, cs.background_window);
					ImGui::PushStyleColor(ImGuiCol_Border, cs.border_color);
					ImGui::PushStyleColor(ImGuiCol_HeaderHovered, cs.widget_hovered_color);
					ImGui::PushStyleColor(ImGuiCol_HeaderActive, cs.widget_active_color);
					ImGui::PushStyleColor(ImGuiCol_Header, cs.widget_color);
					if (ImGui::Begin((id).c_str(), NULL, window_flags)) {
						ImGui::PushFont(fonts[font][48]);
						if(text != "Which file do you want to commit...? ") {
							ImGui::TextWrapped("%s", text.c_str());
						} else if(entries->size() == 1) {
							ImGui::TextWrapped("%s \nThere is %d File ready for commit:", text.c_str(), (uint) entries->size());
						} else {
							ImGui::TextWrapped("%s \nThere are %d Files ready for commit:", text.c_str(), (uint) entries->size());

						}
						for (auto& entry : *entries) {
							if(entry.second->deleted) continue;
							if (ImGui::CollapsingHeader(entry.second->get_collapse_header().c_str())) {
								ImGui::SetNextWindowFocus();
								ImGui::PushFont(GUI::get_font(GUI::FONT::RETRO, 32));
								ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2);
								ImGui::PushStyleColor(ImGuiCol_Button, cs.transparency);
								ImGui::PushStyleColor(ImGuiCol_ButtonActive, cs.widget_active_color);
								ImGui::PushStyleColor(ImGuiCol_ButtonHovered, cs.widget_hovered_color);
								if(entry.second->draw()) clicked = to_string(entry.first);
								if(clicked != "not_clicked") cout << clicked << endl;
								ImGui::PopStyleVar(1);
								ImGui::PopStyleColor(3);
								ImGui::PopFont();
							}
						}
						ImGui::PopFont();
					}
					ImGui::End();
					ImGui::PopStyleVar(2);
					ImGui::PopStyleColor(5);
				}

				bool isClicked() {
					return clicked != "not_clicked";
				}

				string getClicked() {
					return clicked;
				}

				bool get_pressed_enter() {
					return false;
				}
		};

	class overlay_t : public container_t {
		public:
			vector<shared_ptr<overlay_object>> children;
			float element_height;
			overlay_t(string id_p, FONT font_p, float element_height_p) : container_t(id_p, true, glm::vec2(0,0), glm::vec2(-1,-1), glm::vec4(1,1,1,1), glm::vec4(1,1,1,1), 0.f, 0.f, font_p, false, "overlay") {
				element_height = element_height_p;
			}

			bool isClicked() {
				return false;
			}

			void draw() {
				if(!active) return;
				ImGui::PushFont(fonts[font][floor(res.y * element_height)]);
				ImGui::SetNextWindowFocus();
				ImGui::SetNextWindowPos(ImVec2(0,0));
				ImGui::SetNextWindowSize(ImVec2(res.x, res.y));
				ImGui::PushStyleColor(ImGuiCol_WindowBg, cs.transparency);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, rounding);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
				ImGui::Begin("Overlay", NULL, background_flags);
				for(auto &child : children) {
					if(!child->active) continue;
					if(child->type == "text") {
						auto p = glm::vec2(child->pos.x * res.x, child->pos.y * res.y);
						if(child->center_aligned) { auto ip = ImGui::CalcTextSize(child->text.c_str()); /* cout << "x: " << ip.x << " , y: " << ip.y << endl; */ p = glm::vec2(std::max(0.f, p.x - 0.5f * ip.x), p.y); /* cout << glm::to_string(p) << endl; */ }
						ImGui::PushStyleColor(ImGuiCol_Text, cs.text_color);
						ImGui::SetCursorScreenPos(ImVec2(p.x, p.y));
						ImGui::PushTextWrapPos(res.x);
						ImGui::TextWrapped("%s", child->text.c_str());
						ImGui::PopTextWrapPos();
						ImGui::PopStyleColor(1);
					}
					if(child->type == "bar") {
						ImGui::PushStyleColor(ImGuiCol_PlotHistogram, cs.widget_active_color);
						ImGui::PushStyleColor(ImGuiCol_Text, cs.text_color);
						ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 1.f);
						ImGui::SetCursorScreenPos(ImVec2(child->pos.x * res.x, child->pos.y * res.y));
						float perc = atof(child->text.c_str());
						ImGui::ProgressBar(perc, ImVec2(10 * element_height * res.y, element_height * res.y));
						ImGui::PopStyleColor(2);
						ImGui::PopStyleVar(1);
					}
				}
				ImGui::End();
				ImGui::PopStyleColor(1);
				ImGui::PopStyleVar(3);
				ImGui::PopFont();
			}

			uint add_element(string type_p, string text_p, glm::vec2 pos_p, bool center_aligned = false) {
				auto p = pos_p == glm::vec2(-1,-1) ? children[children.size() - 1]->pos + glm::vec2(0, element_height) : pos_p;
				children.push_back(make_shared<overlay_object>(p, text_p, true, type_p, center_aligned));
				return children.size() - 1;
			}

			void edit_element_text(uint id_child, string new_text) {
				children[id_child]->text = new_text;
			}

			bool get_pressed_enter() {
				return false;
			}

			string getClicked() {
				return "none";
			}
	};

	class window_t : public container_t {
		public:
			int window_flags;
			pixel_padding pad;
			struct progressbar_t {
				int progressbar_id = -1;
				float percent = 0.f;
				bool has_bar = false;
			};
			bool has_insertion_field = false;
			progressbar_t bar;
			float window_padding = 0.f;
			bool has_wrapped_text;
			bool enter = false;
			string text_for_insertion = "";
			char buffer[2048];
			bool transparent = false;

			window_t(string id_p, bool active_p, glm::vec2 pos_p, glm::vec2 dim_p, glm::vec4 color_p, glm::vec4 border_col_p, float border_size_p, float rounding_p, int window_flags_p, pixel_padding pad_p, FONT font_p, bool isChild_p, ImGuiWindowFlags_ remove_flags = ImGuiWindowFlags_None) : container_t(id_p, active_p, pos_p, dim_p, color_p, border_col_p, border_size_p, rounding_p, font_p, isChild_p, "window_no_text") {
				window_flags = window_flags_p  & ~(remove_flags);
				pad = pad_p;
				has_wrapped_text = false;
				buffer[0] = '\0';
			}

			bool isClicked() {
				return 0;
			}

			void add_insertion_field(string text_for_insertion_p) {
				window_flags = window_flags & (~ImGuiWindowFlags_NoMouseInputs);
				text_for_insertion = text_for_insertion_p;
				has_insertion_field = true;
			}

			void add_wrapped_text(float p = 0.f, string t = "Sample empty wrapped text that needs to be a bit long to be a really good example, LMAOOOOOOOOO") {
				window_padding = p;
				has_wrapped_text = true;
				text = t;
			}

			void add_progressbar(int i) {
				bar.has_bar = true;
				bar.progressbar_id = i;
				bar_percentage.push_back(1.f);
				bar_percentage_old.push_back(1.f);
				bar_percentage_new.push_back(1.f);
				bar_char_names.push_back("Empty Char Name");
			}

			string get_inserted_text() {
				return string(buffer);
			}

			bool get_pressed_enter() {
				return enter;
			}

			string getClicked() {
				return "none";
			}

			void draw() {
				if(!active) return;
				// cout << "Now Drawing: " << id << " and I am " << (isChild ? "" : "not ") << "a Child" << endl;
				// cout << "My pos: " << glm::to_string(pos) << endl;
				ImVec2 posVec = ImVec2(pos.x * res.x + border_size * 0.5f + pad.front_pad.x, (pos.y * res.y) + border_size * 0.5f + pad.front_pad.y);
				ImVec2 dimVec = ImVec2((dim.x * res.x) - border_size - (pad.back_pad.x + pad.front_pad.x), (dim.y * res.y) - border_size - (pad.back_pad.y + pad.front_pad.y));
				ImGui::SetNextWindowFocus();
				ImGui::SetNextWindowPos(posVec);
				ImGui::SetNextWindowSize(dimVec);
				ImGui::PushFont(fonts[font][48]);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(window_padding, window_padding));
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, border_size);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, rounding);
				ImGui::PushStyleColor(ImGuiCol_WindowBg, transparent ? cs.transparency : cs.background_window);
				ImGui::PushStyleColor(ImGuiCol_Border, cs.border_color);
				ImGui::Begin(id.c_str(), NULL, window_flags);
				enter = false;
				if(bar.has_bar) {
					float bar_height = 50.f;
					float bar_width = dimVec.x * 0.8;
					float fsize = min(bar_height, ImGui::GetBestCalcTextSize((bar_char_names[bar.progressbar_id] + " (69kb)").c_str(), bar_width));
					ImGui::PushFont(fonts[FONT::RETRO][floor(fsize)]);
					ImGui::SetCursorScreenPos(ImVec2(posVec.x + (dimVec.x - bar_width)/2.f, posVec.y + 0.1 * dimVec.y));
					if(!bar.progressbar_id) ImGui::Text("%s", bar_char_names[bar.progressbar_id].c_str());
					else ImGui::Text("%s (%dkb)", bar_char_names[bar.progressbar_id].c_str(), current_file->size);
					ImVec4 bar_col = ImVec4(1 - bar_percentage[bar.progressbar_id], bar_percentage[bar.progressbar_id], 0,1);
					ImGui::PushStyleColor(ImGuiCol_PlotHistogram, bar_col);
					ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.f,1.f,1.f,0.5f));
					ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 1.f);
					ImGui::PushFont(fonts[FONT::PLAYER_STATS_FONT][bar_height]);
					// ImGui::SetCursorScreenPos(ImVec2((posVec.x + dim.x - bar_width)/2.f, posVec.y));
					ImGui::SetCursorScreenPos(ImVec2(posVec.x + (dimVec.x - bar_width)/2.f, posVec.y + dimVec.y - bar_height - 0.2 * dimVec.y));
					//				cout << "Bar percent: " << bar_percentage[bar.progressbar_id] << endl;
					ImGui::ProgressBar(bar_percentage[bar.progressbar_id], ImVec2(bar_width, bar_height));
					ImGui::PopStyleColor(2);
					ImGui::PopStyleVar();
					ImGui::PopFont();
					ImGui::PopFont();
				} else if(has_wrapped_text) {
					ImGui::PushFont(fonts[FONT::RETRO][font_size_optional == 0 ? glm::clamp((float) floor(0.3 * dimVec.y), 6.f, 64.f) : font_size_optional]);
					ImGui::TextWrapped("%s", text.c_str());
					ImGui::PopFont();
				} else if(has_insertion_field) {
					ImGui::PushItemWidth(dimVec.x * (4.f/6.f));
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
					// ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
					ImGui::PushStyleColor(ImGuiCol_Border, cs.border_color);
					ImGui::PushStyleColor(ImGuiCol_Text, cs.text_color);
					ImGui::PushStyleColor(ImGuiCol_FrameBg, cs.transparency);
					ImGui::SetCursorScreenPos(ImVec2(posVec.x + (dimVec.x)/6.f, posVec.y + 0.2 * dimVec.y));
					enter = ImGui::InputTextWithHint("", text_for_insertion.c_str(), buffer, 2047, ImGuiInputTextFlags_EnterReturnsTrue);
					ImGui::PopStyleVar(1);
					ImGui::PopStyleColor(3);
				}
				ImGui::End();
				ImGui::PopFont();
				ImGui::PopStyleColor(2);
				ImGui::PopStyleVar(3);
			}
	};

	class button_t : public container_t {
		public:
			glm::vec4 color_hovered;
			glm::vec4 color_active;
			int bg_window;
			bool clicked;
			bool hovered_last_time = false;
			function<void()> hovered_callback;


			button_t(string id_p, string text_p, bool active_p, glm::vec2 pos_p, glm::vec2 dim_p, glm::vec4 color_p, glm::vec4 color_hovered_p, glm::vec4 color_active_p, glm::vec4 border_col_p, float border_size_p, float rounding_p, FONT font_p, function<void()> hovered_callback_p = 0) : container_t(id_p, active_p, pos_p, dim_p, color_p, border_col_p, border_size_p, rounding_p, font_p, false, text_p){
				color_hovered = color_hovered_p;
				color_active = color_active_p;
				bg_window = create_window(pos_p, dim_p, glm::vec4(0,0,0,0), border_col_p, border_size, background_flags, rounding, pixel_padding{glm::ivec2(0,0), glm::ivec2(0,0)}, font_p, true);
				(dynamic_pointer_cast<GUI::window_t>(Screen::containers[bg_window]))->transparent = true;
				hovered_callback = hovered_callback_p;
			}

			bool isClicked() {
				return clicked;
			}

			void draw() {
				if(active) Screen::containers[bg_window]->draw();
				if(!active) return;
				ImGui::SetNextWindowFocus();
				ImGui::SetNextWindowPos(ImVec2(pos.x * res.x, pos.y * res.y));
				ImGui::SetNextWindowSize(ImVec2(dim.x * res.x, dim.y * res.y));
				// ImGui::SetWindowFontScale(2.f);
				if(active) ImGui::PushStyleColor(ImGuiCol_WindowBg, cs.widget_color);
				else ImGui::PushStyleColor(ImGuiCol_WindowBg, cs.background_window_inactive);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, rounding);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
				float neededFontSize = ImGui::GetBestCalcTextSize(text.c_str(), dim.x * res.x * 0.9);
				ImGui::PushFont(fonts[font][text == "PUSH!" ? 128 : std::max(6.f,floor(neededFontSize))]);
				ImGui::Begin(("Container:" + id).c_str(), NULL, ImGuiWindowFlags_NoDecoration);
				ImGui::PushStyleColor(ImGuiCol_Button, cs.widget_color);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, cs.widget_hovered_color);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, cs.widget_active_color);
				clicked = ImGui::Button(text.c_str(), ImVec2(dim.x * res.x, dim.y * res.y));
				bool hovered_now = ImGui::IsItemHovered();
				if(hovered_callback && hovered_now && hovered_now != hovered_last_time) { hovered_callback(); }
				hovered_last_time = hovered_now;
				ImGui::PopStyleColor(3);
				ImGui::End();
				ImGui::PopFont();
				ImGui::PopStyleColor();
				ImGui::PopStyleVar(3);
			}

			bool get_pressed_enter() {
				return false;
			}

			string getClicked() {
				return "none";
			}
	};


	void init_GUI() {
		// f[0] = 0.f;
		// f[1] = 0.f;

			ImFontConfig config;
			config.OversampleH = 3;
			config.OversampleV = 3;
			fonts[FONT::RETRO][128] = ImGui::GetIO().Fonts->AddFontFromFileTTF((concat(EXECUTABLE_DIR, font_paths[FONT::RETRO])).c_str(), 128, &config);
		for(uint i = 64; i >= 6; i--) {
			for(auto &f : font_paths) {
				if(i%4) {
					fonts[f.first][i] = fonts[f.first][i+1];
					continue;
				}
				ImFontConfig config;
				config.OversampleH = 3;
				config.OversampleV = 3;
				fonts[f.first][i] = ImGui::GetIO().Fonts->AddFontFromFileTTF((concat(EXECUTABLE_DIR, f.second)).c_str(), i, &config);
			}
			// ImFontConfig configComicsans;
			// configComicsans.OversampleH = 3;
			// configComicsans.OversampleV = 3;
			// fonts[FONT::PLAYER_STATS_FONT][i] = ImGui::GetIO().Fonts->AddFontFromFileTTF((concat(EXECUTABLE_DIR, "render-data/fonts/fixedsys.ttf")).c_str(), i, &configComicsans);
		}
		// cout << "INIT DEBUG" << endl;
		// for(auto &font : fonts) {
		// 	cout << font.first << endl;
		// 	cout << font.second->GetDebugName();
		// }
		// for(auto &f : font_paths) {
		// 	fonts[f.first] = fonts[FONT::RETRO];
		// }
	}

	bool isClicked(uint container_id) {
		return Screen::containers.at(container_id)->isClicked();
	}

	glm::vec2 pixels_to_percent(glm::ivec2 v, glm::ivec2 resolution) {
		return glm::vec2(v)/glm::vec2(resolution);
	}

	// glm::vec2 pixels_to_percent(glm::vec2 v, glm::ivec2 resolution) {
	// 	return v/glm::vec2(resolution);
	// }

	uint create_window(glm::vec2 pos, glm::vec2 dim, glm::vec4 color, glm::vec4 border_col, float border_size, int window_flags, float rounding, pixel_padding pads, FONT font_p, bool isChild_p, ImGuiWindowFlags_ remove_flags) {
		Screen::containers[container_ids] = make_shared<window_t>(to_string(container_ids), true, pos, dim, color, border_col, 2 * border_size, rounding, window_flags, pads, font_p, isChild_p, remove_flags);
		return container_ids++;
	}

	uint create_overlay(float element_height) {
		Screen::containers[container_ids] = make_shared<overlay_t>(to_string(container_ids), FONT::RETRO, element_height);
		return container_ids++;
	}

	template<typename T>
	uint create_collapse_window(glm::vec2 pos, glm::vec2 dim, glm::vec4 color, glm::vec4 border_col, float border_size, int window_flags, float rounding, pixel_padding pads, FONT font_p, bool isChild) {
		Screen::containers[container_ids] = make_shared<collapse_window_t<T>>(to_string(container_ids), true, pos, dim, color, border_col, 2 * border_size, rounding, window_flags, pads, font_p, isChild);
		return container_ids++;

	}

	uint create_button(string text, glm::vec2 pos, glm::vec2 dim, glm::vec4 color, glm::vec4 color_hovered, glm::vec4 color_active, glm::vec4 border_col, float border_size, float rounding, FONT font_p, function<void()> hovered_callback_p) {
		uint my_id = container_ids++;
		Screen::containers[my_id] = make_shared<button_t>(to_string(my_id), text, true, pos, dim, color, color_hovered, color_active, border_col, border_size, rounding, font_p, hovered_callback_p);
		return my_id;
	}

	// uint create_button(glm::vec2 pos, glm::vec2 dim, glm::vec4 color, glm::vec4

	void draw() {
		res = Context::resolution();
		for(auto &container : Screen::containers) {
			if(!container.second->isChild) container.second->draw();
		}
	}

	void draw(shared_ptr<vector<uint>> ids) {
		res = Context::resolution();
		// cout << "Start" << endl;
		for(auto &id : *ids) {
			// cout << id << endl;
			Screen::containers[id]->draw();
		}
		// cout << "End" << endl;
	}

	void setSize(int id, glm::vec2 s) {
		if(id == -1) return;
		Screen::containers[id]->dim = s;
	}

	void setPos(int id, glm::vec2 p) {
		if(id == -1) return;
		Screen::containers[id]->pos = p;
	}

	void set_percentage(int index, float percentage) {
		bar_percentage_new[index] = glm::clamp(percentage, 0.0f, 1.0f);
	}
	void reset_percentage() {
		for(auto &bar : bar_percentage) {
			bar = 1.f;
		}
		for(auto &bar : bar_percentage_new) {
			bar = 1.f;
		}
		for(auto &bar : bar_percentage_old) {
			bar = 1.f;
		}
	}

	void set_char_name(int index, string char_name) {
		bar_char_names[index] = char_name;
	}

	void draw_loading_screen(float percent, string text, ImVec4 color, ImVec4 color_bg) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::SetNextWindowPos(ImVec2(0,0));
		auto r = Context::resolution();
		ImGui::SetNextWindowSize(ImVec2(r.x, r.y));
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, cs.loading_bar_color);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, cs.loading_bar_background_color);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, color_bg.x == -1 ? cs.background_window : color_bg);
		ImGui::Begin("LOADING", NULL, ImGuiWindowFlags_NoDecoration);
		ImGui::PushFont(fonts[FONT::RETRO][64]);
		ImVec2 text_dim = ImGui::CalcTextSize(text.c_str());
		ImGui::SetCursorScreenPos(ImVec2(r.x/2 - 0.5 * text_dim.x, r.y/2 - text_dim.y * 0.5f));
		ImGui::PushStyleColor(ImGuiCol_Text, cs.text_color);
		ImGui::Text("%s", text.c_str());
		ImGui::SetCursorScreenPos(ImVec2(r.x/4, r.y/2 + text_dim.y * 0.5f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0,0));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
		ImGui::ProgressBar(percent, ImVec2(r.x/2, 80));
		ImGui::PopStyleVar(1);
		ImGui::PopFont();
		ImGui::PopStyleColor(2);
		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(2);
	}

}

void Screen::generate_health_bar_screen() {
	uint bg = GUI::create_window(glm::vec2(0.0, 0.0), glm::vec2(0.35f, 0.2f), glm::vec4(0,0,0,0.8), glm::vec4(0,1,0,1), 2.f, background_flags, 0.f, GUI::pixel_padding {glm::ivec2(10,10), glm::ivec2(10,10) });
	screen_objects->push_back(bg);
	(dynamic_pointer_cast<GUI::window_t>(containers[bg]))->add_progressbar(1);
	bg = GUI::create_window(glm::vec2(0.65, 0.46), glm::vec2(0.35f, 0.2f), glm::vec4(0,0,0,0.8), glm::vec4(0,1,0,1), 2.f, background_flags, 0.f, GUI::pixel_padding {glm::ivec2(10,10), glm::ivec2(10,10) });
	(dynamic_pointer_cast<GUI::window_t>(containers[bg]))->add_progressbar(0);
	screen_objects->push_back(bg);
}

void Screen::generate_overlay() {
	float element_height = 0.04f;
	uint bg = GUI::create_overlay(element_height);
	screen_objects->push_back(bg);
	shared_ptr<GUI::overlay_t> overlay = (dynamic_pointer_cast<GUI::overlay_t>(containers[bg]));
	files_text_child_id = overlay->add_element("text", "Progress: ", glm::vec2(0,0));
	overlay->add_element("bar", to_string(0.5f), glm::vec2(-1, -1));
	overlay->add_element("text", "No Files generated", glm::vec2(-1, -1));
	overlay->add_element("text", "No Files generated", glm::vec2(-1, -1));
	overlay_hint_id = overlay->add_element("text", "", glm::vec2(0.5, 0.35), true);
}

void Screen::update_overlay() {
	shared_ptr<GUI::overlay_t> overlay = (dynamic_pointer_cast<GUI::overlay_t>(containers[(*screen_objects)[0]]));
	float percentage = ((float)Files::commited.size()) / (Files::commited.size() + Files::collected.size() + Files::files.size());
	current_save->data->playerlevel= round(percentage * 100);
	overlay->edit_element_text(files_text_child_id+1, to_string(percentage));
	overlay->edit_element_text(files_text_child_id+2, "Files collected: " + to_string(Files::collected.size()));
	overlay->edit_element_text(files_text_child_id+3, "Files commited: " + to_string(Files::commited.size()));
	overlay->edit_element_text(overlay_hint_id, "");
	if(Checkpoints::show_overlay_allowed()) { overlay->edit_element_text(overlay_hint_id, "Press E to open the checkpoint menu");}
}

void Screen::set_overlay_hint(string new_string) {
	(dynamic_pointer_cast<GUI::overlay_t>(containers[(*screen_objects)[0]]))->children[overlay_hint_id]->text = new_string;
}

void Screen::generate_fight_screen() {
	cout << "Screen \"Fight\" wird erstellt." << endl;
	uint bg = GUI::create_window(glm::vec2(0.0, 0.66), glm::vec2(1.f, 0.34f), glm::vec4(0,0,0,0.8f), glm::vec4(0,1,0,1), 2.f, background_flags, 0.f,  GUI::pixel_padding {glm::ivec2(10,10), glm::ivec2(10,10)}); // Main Window on Bottom
	screen_objects->push_back(bg);
	bg = GUI::create_button("<", glm::vec2(0.648, 0.85), glm::vec2(0.275,0.1), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::ARROWS, Jukebox::play_beep); // Arrow up Button
	screen_objects->push_back(bg);
	dec_page_button = bg;
	bg = GUI::create_button(">", glm::vec2(0.648, 0.71), glm::vec2(0.275,0.1), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::ARROWS, Jukebox::play_beep); // Arrow down Button
	screen_objects->push_back(bg);
	inc_page_button = bg;
	bg = GUI::create_button("Sample1", glm::vec2(0.04,0.71), glm::vec2(0.275,0.1), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); //Attack Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	bg = GUI::create_button("Sample2", glm::vec2(0.04,0.85), glm::vec2(0.275,0.1), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); //Attack Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	bg = GUI::create_button("Sample3", glm::vec2(0.344,0.71), glm::vec2(0.275,0.1), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); //Attack Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	bg = GUI::create_button("Sample4", glm::vec2(0.344,0.85), glm::vec2(0.275,0.1), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); //Attack Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	debugs[0] = bg;
	f[0] =0.65;
}

void Screen::generate_text_screen() {
	generate_text_and_okay_screen(false);
}

void Screen::generate_text_and_okay_screen(bool okay) {
	uint bg = GUI::create_window(glm::vec2(0.0, 0.66), glm::vec2(1.f, 0.34f), glm::vec4(0,0,0,0.8f), glm::vec4(0,1,0,1), 2.f, background_flags, 0.f,  GUI::pixel_padding {glm::ivec2(10,10), glm::ivec2(10,10)}); // Main Window on Bottom
	screen_objects->push_back(bg);
	(dynamic_pointer_cast<GUI::window_t>(containers[bg]))->add_wrapped_text();
	if(!okay) return;
	bg = GUI::create_button("Okay", glm::vec2(1.f-0.22,1.f-0.1), glm::vec2(0.2,0.08), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); // Okay Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
}

void Screen::generate_victory_screen() {
	uint bg = GUI::create_button("PUSH!", glm::vec2(0.1, 0.2), glm::vec2(0.8, 0.2), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); // New Game Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	float button_height = 0.08f;
	bg = GUI::create_button("Back to main menu", glm::vec2(0.02, button_height * 7.f), glm::vec2(0.2, button_height), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); // Back to Main menu Button
	containers[bg]->active = false;
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
}

void Screen::draw_victory_screen() {
	draw();
	static float push_percentage = 0.f;
	static bool push_last_time = false;
	push_percentage = glm::clamp((float) pow(push_percentage, 1.02), 0.f, 1.f);
	if(containers[(*Buttons)[0]]->isClicked() && !push_last_time) {
		push_percentage += 0.008 + 0.1 * ceil(1 - push_percentage - 0.1) + 0.05 * (1 - push_percentage) + pow(0.3 * (1 - push_percentage), 2);
	}
	if(containers[(*Buttons)[1]]->active && containers[(*Buttons)[1]]->isClicked()) {
		Jukebox::pause_static_sound(Jukebox::sound::world_music);
		won = false;
		game_mode = "start_screen";
	}
	push_last_time = containers[(*Buttons)[0]]->isClicked();
	GUI::draw_loading_screen(push_percentage, push_percentage < 1.f ? "PUSH YOUR FILES AS HARD AS YOU CAN!" : "FINALLY YOU COMMITED YOUR FILES!\nTime for the next FINAL COMMIT", ImVec4(1,1,1,1), ImVec4(1,1,1,0));
	if(push_percentage >= 1.f) {
		containers[(*Buttons)[1]]->active = true;
		draw_shader(victory_shader);
	}
}

void Screen::generate_start_screen() {
	cout << "generating fight screen" << endl;
	// background_image = make_texture("background_image", "render-data/images/start_screen_bg.png");
	float button_height = 0.08;
	uint bg = GUI::create_button("New Game_sample", glm::vec2(0.02, button_height), glm::vec2(0.2, button_height), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); // New Game Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	bg = GUI::create_button("Load Game_sample", glm::vec2(0.02, button_height * 2.5), glm::vec2(0.2, button_height), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); // Load Game Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	bg = GUI::create_button("How to play_sample", glm::vec2(0.02, button_height * 4), glm::vec2(0.2, button_height), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); // How to play Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	bg = GUI::create_button("Exit_sample", glm::vec2(0.02, button_height * 5.5), glm::vec2(0.2, button_height), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); // Exit Game Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	start_screen_right_pos = glm::vec2(0.24, button_height);
	start_screen_right_dim = glm::vec2(1 - start_screen_right_pos.x - 0.02, 1 - 2 * button_height);
	bg = GUI::create_window(start_screen_right_pos, start_screen_right_dim, glm::vec4(0,0,0,0.8f), glm::vec4(0,1,0,1), 2.f, background_flags, 0.f,  GUI::pixel_padding {glm::ivec2(0,0), glm::ivec2(0,0)}); // Insert name window
	screen_objects->push_back(bg);
	insert_name_window_ids.push_back(bg);
	(dynamic_pointer_cast<GUI::window_t>(containers[bg]))->add_insertion_field("Insert name (Optional)");
	bg = GUI::create_button("START", glm::vec2(start_screen_right_pos.x + 0.5 * start_screen_right_dim.x - 0.1, 0.4), glm::vec2(0.2, button_height), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep);
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	insert_name_window_ids.push_back(bg);
	bg = GUI::create_window(start_screen_right_pos, start_screen_right_dim, glm::vec4(0,0,0,0.8f), glm::vec4(0,1,0,1), 2.f, background_flags, 0.f,  GUI::pixel_padding {glm::ivec2(0,0), glm::ivec2(0,0)}, GUI::FONT::RETRO, false, (ImGuiWindowFlags_) (ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs)); // How to play window
	screen_objects->push_back(bg);
	containers[bg]->font_size_optional = 32;

	std::ifstream t("render-data/text/how_to.txt");
	std::string how_to;
	t.seekg(0, std::ios::end);
	how_to.reserve(t.tellg());
	t.seekg(0, std::ios::beg);
	how_to.assign((std::istreambuf_iterator<char>(t)),
			   std::istreambuf_iterator<char>());

	(dynamic_pointer_cast<GUI::window_t>(containers[bg]))->add_wrapped_text(10.f, how_to);
	containers[bg]->active = false;
	how_to_play_window_ids.push_back(bg);
	bg = GUI::create_collapse_window<shared_ptr<save_t>>(start_screen_right_pos, start_screen_right_dim, glm::vec4(0,0,0,0.8f), glm::vec4(0,1,0,1), 2.f, background_flags, 0.f,  GUI::pixel_padding {glm::ivec2(0,0), glm::ivec2(0,0)}, GUI::FONT::RETRO, false);
	(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<save_t>>>(containers[bg]))->entries = save_t::init_saves();
	(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<save_t>>>(containers[bg]))->text = "Select your savegame ... ";
	load_game_collapse_window_ids.push_back(bg);
	screen_objects->push_back(bg);
	use_texts_input({"New Game", "Load Game", "How to play", "Exit", "Start"}, true);
}

void Screen::generate_checkpoint_screen() {
	float button_height = 0.08;
	uint bg = GUI::create_button("Files..._init", glm::vec2(0.02, button_height), glm::vec2(0.2, button_height), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); // Files... Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	bg = GUI::create_button("Save Game", glm::vec2(0.02, button_height * 2.5), glm::vec2(0.2, button_height), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); // Save Game Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	bg = GUI::create_button("Load Game", glm::vec2(0.02, button_height * 4), glm::vec2(0.2, button_height), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); // Load Game Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	bg = GUI::create_button("Back to game", glm::vec2(0.02, button_height * 5.5), glm::vec2(0.2, button_height), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); // Back to Game Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	bg = GUI::create_button("Back to main menu", glm::vec2(0.02, button_height * 7.f), glm::vec2(0.2, button_height), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); // Back to Main menu Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	bg = GUI::create_button("Exit Game", glm::vec2(0.02, button_height * 8.5f), glm::vec2(0.2, button_height), glm::vec4(0,1,0,0.2f), glm::vec4(0,0.8f,0,0.5), glm::vec4(0, 1.f, 0, 0.5), glm::vec4(0,1,0,1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep); // Exit Game Button
	screen_objects->push_back(bg);
	Buttons->push_back(bg);
	bg = GUI::create_collapse_window<shared_ptr<Files::file>>(start_screen_right_pos, start_screen_right_dim, glm::vec4(0,0,0,0.8f), glm::vec4(0,1,0,1), 2.f, background_flags, 0.f,  GUI::pixel_padding {glm::ivec2(0,0), glm::ivec2(0,0)}, GUI::FONT::RETRO, false);
	(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<Files::file>>>(containers[bg]))->entries = Files::make_collected_files_map();
	(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<Files::file>>>(containers[bg]))->text = "Which file do you want to commit...? ";
	load_files_collapse_window_ids.push_back(bg);
	screen_objects->push_back(bg);
	bg = GUI::create_collapse_window<shared_ptr<save_t>>(start_screen_right_pos, start_screen_right_dim, glm::vec4(0,0,0,0.8f), glm::vec4(0,1,0,1), 2.f, background_flags, 0.f,  GUI::pixel_padding {glm::ivec2(0,0), glm::ivec2(0,0)}, GUI::FONT::RETRO, false);
	(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<save_t>>>(containers[bg]))->entries = save_t::update_saves();
	(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<save_t>>>(containers[bg]))->text = "Select your savegame ... ";
	load_game_collapse_window_ids.push_back(bg);
	screen_objects->push_back(bg);
	use_texts_input({"Files...", "Save Game", "Load Game", "Back to game", "Back to main menu", "Exit Game"}, true);
}

void Screen::update_checkpoint_screen() {
	(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<Files::file>>>(containers[(load_files_collapse_window_ids)[0]]))->entries = Files::make_collected_files_map();
	// for(auto &button_id : *Buttons) {
	// 	auto button = containers[button_id];
	// 	if(!button->isClicked()) continue;
	// }
}

bool Screen::draw_healthbar_anim(bool start, int player) {
	if(start) { healthbar_timer.restart(); }
	double drop_percentage = healthbar_timer.look()/(healthbar_time * 1000);
	if(drop_percentage > 1.f) {
		GUI::bar_percentage_old[player] = GUI::bar_percentage_new[player];
		GUI::bar_percentage[player] = GUI::bar_percentage_new[player];
		return false;
	}
	GUI::bar_percentage[player] = GUI::bar_percentage_old[player] + drop_percentage * (GUI::bar_percentage_new[player] - GUI::bar_percentage_old[player]);

	return true;
}


Screen::Screen(string name_p) : name(name_p) {
	screen_objects = make_shared<vector<uint>>();
	Buttons = make_shared<vector<uint>>();
	if(name_p == "empty") {
		return;
	} else if(name_p == "fight") {
		generate_fight_screen();
	} else if(name_p == "text") {
		generate_text_screen();
	} else if(name_p == "text_and_okay") {
		generate_text_and_okay_screen();
	} else if(name_p == "start") {
		generate_start_screen();
	} else if(name_p == "health_bar") {
		generate_health_bar_screen();
	} else if(name_p == "overlay") {
		generate_overlay();
	} else if(name_p == "checkpoint") {
		generate_checkpoint_screen();
	} else if(name_p == "victory") {
		generate_victory_screen();
	}
}

void Screen::draw() {
	// ImGui::Begin("Debugger");
	// ImGui::DragFloat2("Pos1", f, 0.001f);
	// ImGui::DragFloat2("Pos2", &f[2], 0.001f);
	// ImGui::DragFloat2("Pos3", &f[4], 0.001f);
	// ImGui::DragFloat2("Pos4", &f[6], 0.001f);
	// ImGui::DragFloat2("Pos5", &f[8], 0.001f);
	// ImGui::End();
	// cout << screen_objects->size() << " Objects to draw" << endl;
	// GUI::setPos(debugs[0], glm::vec2(f[0], f[1]));
	// GUI::setSize(debugs[0], glm::vec2(f[2], f[3]));
	// GUI::setPos(6, glm::vec2(f[2], f[3]));
	// GUI::setPos(7, glm::vec2(f[4], f[5]));
	// GUI::setPos(8, glm::vec2(f[4], f[5]));
	if(background_image) {
		// blit(background_image);
	}
	GUI::draw(screen_objects);
	ImGui::SetWindowFocus(NULL);
	if(dec_page_button < 0 || inc_page_button < 0) return;
	shared_ptr<GUI::button_t> button = dynamic_pointer_cast<GUI::button_t>(containers[dec_page_button]);
	if(button->active && button->isClicked()) set_scroll_page(scroll_page - 1);
	else {
		button = dynamic_pointer_cast<GUI::button_t>(containers[inc_page_button]);
		if(button->active && button->isClicked()) set_scroll_page(scroll_page + 1);
	}
}

void Screen::draw_checkpoint_screen() {
	static string right_pane = "file_selection";
	draw();
	for(auto &elem : load_game_collapse_window_ids) {
		containers[elem]->active = false;
	}
	for(auto &elem : load_files_collapse_window_ids) {
		containers[elem]->active = false;
	}
	if(right_pane == "file_selection") {
		for(auto &elem : load_files_collapse_window_ids) {
			containers[elem]->active = true;
		}
	}
	if(right_pane == "load_game") {
		for(auto &elem : load_game_collapse_window_ids) {
			containers[elem]->active = true;
		}
	}


	string clicked_string = get_clicked_text_id();
	if(right_pane == "file_selection") {
		auto tmp = containers[load_files_collapse_window_ids[0]];
		if(tmp->isClicked()) {
			current_save->save_me(true);
			save_t::update_saves();
			string click = tmp->getClicked();
				cerr << endl << endl << click << endl << endl;
			current_file = (*(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<Files::file>>>(tmp))->entries)[atol(click.c_str())];

			cout << "shutting down 3D World." << endl << "starting battle" << endl;
			World3DHandler::deactivate_checkpoint_overlay(false);
			World3DHandler::shutdown();
			Jukebox::pause_static_sound(Jukebox::sound::world_music_damped);
			game_mode = "battle";
		}
	}
	if(right_pane == "load_game") {
		auto tmp = containers[load_game_collapse_window_ids[0]];
		if(tmp->isClicked()) {
			current_save = (*(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<save_t>>>(tmp))->entries)[atol(tmp->getClicked().c_str())];
			World3DHandler::reboot();
			World3DHandler::deactivate_checkpoint_overlay();
			cout << "hi?" << endl;
		}
	}
	if(clicked_string == "no_attack") return;

	if(clicked_string == "Exit Game") {
		Context::stop_running();
		World3DHandler::shutdown();
	}

	if(clicked_string == "Files...") {
		right_pane = "file_selection";
	}
	else if(clicked_string == "Load Game") {
		(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<save_t>>>(containers[load_game_collapse_window_ids[0]]))->entries = save_t::update_saves();
		cout << "updated saves" << endl;
		right_pane = "load_game";
	}
	else if(clicked_string == "Save Game") {
		right_pane = "load_game";
		use_texts_input({"Files...", "Saved!", "Load Game", "Back to game", "Back to main menu", "Exit Game"}, true);
		current_save->save_me();
		cout << "saved current save" << endl;
		(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<save_t>>>(containers[load_game_collapse_window_ids[0]]))->entries = save_t::update_saves();
	}
	else if(clicked_string == "Back to game") {
		World3DHandler::deactivate_checkpoint_overlay();
	}
	else if(clicked_string == "Back to main menu") {
		World3DHandler::deactivate_checkpoint_overlay();
		(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<save_t>>>(containers[start_screen->load_game_collapse_window_ids[0]]))->entries = save_t::update_saves();
		game_mode = "start_screen";
		World3DHandler::shutdown();
	}
	// auto tmp = containers[load_game_collapse_window_ids[0]];
	// if(right_pane == "load_game" && tmp->isClicked()) {

	// 	current_save = (*(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<save_t>>>(tmp))->entries)[atol(tmp->getClicked().c_str())];
	// 	game_mode = "world3d";
	// 	return;
	// }
	// cout << "received clicked_string: " << clicked_string << endl;
	// if(clicked_string == "Files..." ) {
	// 	right_pane = "insert_name";
	// } else if(clicked_string == "Save Game") {
	// 	right_pane = "load_game";
	// } else if(clicked_string == "Load Game") {
	// 	right_pane = "load_game";
	// }
}

void Screen::draw_start_screen() {
	static string right_pane = "insert_name";
	draw();
	for(auto &elem : how_to_play_window_ids) {
		containers[elem]->active = false;
	}
	for(auto &elem : insert_name_window_ids) {
		containers[elem]->active = false;
	}
	for(auto &elem : load_game_collapse_window_ids) {
		containers[elem]->active = false;
	}
	if(right_pane == "htp") {
		for(auto &elem : how_to_play_window_ids) {
			containers[elem]->active = true;
		}
	}
	bool enter = false;
	if(right_pane == "insert_name") {
		for(auto &elem : insert_name_window_ids) {
			enter |= containers[elem]->get_pressed_enter();
		}
		for(auto &elem : insert_name_window_ids) {
			containers[elem]->active = true;
		}
	}
	if(right_pane == "load_game") {
		(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<save_t>>>(containers[load_game_collapse_window_ids[0]]))->entries = save_t::update_saves();
		for(auto &elem : load_game_collapse_window_ids) {
			containers[elem]->active = true;
		}
	}
	string clicked_string = get_clicked_text_id();
	if(clicked_string == "Start" || enter) {
		inserted_name = (dynamic_pointer_cast<GUI::window_t>(containers[insert_name_window_ids[0]]))->get_inserted_text();
		if(inserted_name == "") {
			inserted_name = "Heinrich";
		}
		// cout << "Inserted Name: " << inserted_name << endl;
		game_mode = "world3d";
		current_save = make_shared<save_t>(inserted_name);
		return;
	}
	auto tmp = containers[load_game_collapse_window_ids[0]];
	if(right_pane == "load_game" && tmp->isClicked()) {

		current_save = (*(dynamic_pointer_cast<GUI::collapse_window_t<shared_ptr<save_t>>>(tmp))->entries)[atol(tmp->getClicked().c_str())];
		game_mode = "world3d";
		return;
	}
	if(clicked_string == "no_attack") return;
	cout << "received clicked_string: " << clicked_string << endl;
	if(clicked_string == "New Game" ) {
		right_pane = "insert_name";
	} else if(clicked_string == "Load Game") {
		right_pane = "load_game";
	} else if(clicked_string == "How to play") {
		right_pane = "htp";
	} else if(clicked_string == "Exit") {
		Context::stop_running();
	}
}

void Screen::set_scroll_page(uint page) {
	scroll_page = page;
//	cout << "Buttons size is: " << Buttons->size() << endl;
	for(uint i = 0; i < Buttons->size(); i++) {
		uint button_index = (*Buttons)[i];
		shared_ptr<GUI::button_t> button = dynamic_pointer_cast<GUI::button_t>(containers[button_index]);
		uint txts_in_index = scroll_page * Buttons->size() + i;
		if( txts_in_index < texts_input.size()) {
			button->active = true;
			button->text = texts_input[txts_in_index];
		} else {
			button->active = false;
		}
	}
	if(dec_page_button < 0 || inc_page_button < 0) return;
	if(page > 0) containers[dec_page_button]->active = true; else containers[dec_page_button]->active = false;
	if(page < ceil(((float)texts_input.size())/Buttons->size()) - 1) containers[inc_page_button]->active = true; else containers[inc_page_button]->active = false;
}

void Screen::use_texts_input(vector<string> texts_p, bool only_buttons) {
	texts_input = texts_p;
	if(only_buttons) {
		for(uint i = 0; i < Buttons->size(); i++) {
			// if(texts_p.size() <= i) continue;
			auto b = (*Buttons)[i];
			containers[b]->text = texts_p.size() <= i ? "" : texts_p[i];
		}
	} else if(name == "text" || name == "text_and_okay") {
		shared_ptr<vector<uint>> vect = screen_objects;
		containers[(*vect)[0]]->text = texts_p.front();
	}
}

std::string Screen::get_clicked_text_id() {
	for(uint i = 0; i < Buttons->size(); i++) {
		uint button_index = (*Buttons)[i];
		if(containers[button_index]->isClicked()) {
			if(texts_input.size() <= i) continue;
			if(texts_input.size() == 0) return "no_texts_input!";
			return texts_input[scroll_page * Buttons->size() + i];
		}
	}
	return "no_attack";
}

void Screen::set_health_percentage(int bar_index, float percent) {
	GUI::bar_percentage_new[bar_index] = glm::clamp(percent, 0.f, 1.f);
}
