#define GLM_ENABLE_EXPERIMENTAL

#include "battlehandler.h"
#include "fighter.h"
#include "effects.h"
#include "../rendering.h"
#include "../skybox.h"
#include "../gui.h"
#include "../files.h"
#include <memory>
#include <iostream>
#include <cppgl/context.h>
#include <cppgl/camera.h>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../save.h"
#include "../jukebox.h"

using namespace std;

shared_ptr<Fighter> BattleHandler::Player, BattleHandler::Enemy;
vector<shared_ptr<Fighter>> BattleHandler::all_enemies;
vector<shared_ptr<Drawelement>> BattleHandler::prototype_arena;
std::string prev_text_to_display;
extern shared_ptr<Skybox> the_skybox;
extern shared_ptr<Shader> bone_shader, sky_shader, sprite_shader, sprite_loading_shader, sprite_normal_shader, sprite_hot_fix_shader, arena_wall_shader, arena_floor_shader, arena_loading_shader;
extern bool debug_gui, super_easy, won;
extern shared_ptr<Files::file> current_file;
extern shared_ptr<save_t> current_save;
extern string game_mode;
extern vector<string> std_moves, std_future_moves;

// static variables
bool BattleHandler::new_in_state = true, BattleHandler::i_wanna_end_me = false, BattleHandler::first_call = true;
std::queue<std::shared_ptr<BattleHandler::state_t>> BattleHandler::states;
std::shared_ptr<BattleHandler::attacks_t> BattleHandler::att_result;
extern std::shared_ptr<Screen> active_screen, fight_screen, text_screen, health_bar_screen, text_and_okay_screen, overlay_screen;

void arena_draw(const std::shared_ptr<Shader> &shader = NULL, float time = -1);

shared_ptr<BattleHandler::attacks_t> BattleHandler::calc_attack_results(string attack_id_of_player) {


	auto all_attacks = make_shared<BattleHandler::attacks_t>();
	int player_speed_bonus = 0;
	if (attack_id_of_player == "HotFix") player_speed_bonus = 100;
	if (Player->stats.speed + player_speed_bonus >= Enemy->stats.speed) {
		Moves::move move_player = Moves::all_moves[attack_id_of_player];
		Moves::single_attack_t atk_1 = move_player.func(Player, Enemy);
		all_attacks->p1_is_player = true;
		all_attacks->attacks.first = atk_1;

		Player->apply_stat_diff(atk_1.stat_change.first);
		Enemy->apply_stat_diff(atk_1.stat_change.second);

		Moves::move move_enemy = Moves::all_moves[Enemy->get_random_move()];
//		 Moves::move move_enemy = Moves::all_moves["Wall of Ugly"];
		Moves::single_attack_t atk_2 = move_enemy.func(Enemy, Player);
		all_attacks->attacks.second = atk_2;

		Enemy->apply_stat_diff(atk_2.stat_change.first);
		Player->apply_stat_diff(atk_2.stat_change.second);
	} else {
		Moves::move move_enemy = Moves::all_moves[Enemy->get_random_move()];
		Moves::single_attack_t atk_1 = move_enemy.func(Enemy, Player);
		all_attacks->p1_is_player = false;
		all_attacks->attacks.first = atk_1;

		Enemy->apply_stat_diff(atk_1.stat_change.first);
		Player->apply_stat_diff(atk_1.stat_change.second);

		Moves::move move_player = Moves::all_moves[attack_id_of_player];
		Moves::single_attack_t atk_2 = move_player.func(Player, Enemy);
		all_attacks->attacks.second = atk_2;

		Enemy->apply_stat_diff(atk_2.stat_change.first);
		Player->apply_stat_diff(atk_2.stat_change.second);
	}
	return all_attacks;
}

string BattleHandler::generate_display_action_text(bool p1) {
	string action_text = "";
	if (p1) {
		action_text += "You used ";
		if (att_result->p1_is_player) {
			action_text += att_result->attacks.first.attack_name;
		} else {
			action_text += att_result->attacks.second.attack_name;
		}
	} else {
		action_text += Enemy->name + " used ";
		if (!att_result->p1_is_player) {
			action_text += att_result->attacks.first.attack_name;
		} else {
			action_text += att_result->attacks.second.attack_name;
		}
	}
	action_text += "!";
	return action_text;
}

string BattleHandler::generate_display_stats_change_text(bool p1) {
	string action_text = "";
	Fighter::stats_t stat_diff_player, stat_diff_enemy;
	Moves::single_attack_t player_atack, enemy_attack;
	if (p1) {
		if (att_result->p1_is_player) {
			player_atack = att_result->attacks.first;
		} else {
			player_atack = att_result->attacks.second;
		}
		stat_diff_player = player_atack.stat_change.first;
		stat_diff_enemy = player_atack.stat_change.second;

		if (stat_diff_enemy.hp != 0) {
			if (stat_diff_enemy.hp > 0) {
				action_text += "You healed " + Enemy->name;
			} else {
				if (player_atack.effective == 1) {
					action_text += "It was very effective!";
				} else if (player_atack.effective == 2) {
					action_text += "It was super effective!!";
				} else if (player_atack.effective == -1) {
					action_text += "It was not very effective...";
				} else if (player_atack.effective == 3) {
					action_text += "A critical hit!!!";
				}
			}
		}
		if (stat_diff_player.hp != 0) {
			action_text += "You ";
			if (stat_diff_player.hp > 0) {
				action_text += "healed yourself for " + to_string(stat_diff_player.hp) + " HP!";
			} else {
				action_text += "damaged yourself!";
			}
		}
	} else {
		if (att_result->p1_is_player) {
			enemy_attack = att_result->attacks.second;
		} else {
			enemy_attack = att_result->attacks.first;
		}
		stat_diff_player = enemy_attack.stat_change.second;
		stat_diff_enemy = enemy_attack.stat_change.first;
		if (stat_diff_player.hp != 0) {
			if (stat_diff_player.hp > 0) {
				action_text += Enemy->name + " healed you!";
			} else {
				if (enemy_attack.effective == 1) {
					action_text += "It was very effective!";
				} else if (enemy_attack.effective == 2) {
					action_text += "It was super effective!!";
				} else if (enemy_attack.effective == -1) {
					action_text += "It was not very effective...";
				} else if (enemy_attack.effective == 3) {
					action_text += "A critical hit!!!";
				}
			}
		}
		if (stat_diff_enemy.hp != 0) {
			action_text += Enemy->name + " ";
			if (stat_diff_enemy.hp > 0) {
				action_text += "healed themself!";
			} else {
				action_text += "damaged themself!";
			}
		}
	}
	if (stat_diff_player.hp == 0) {
		if (stat_diff_player.speed > 0) {
			action_text += "Your speed increased!\n";
		} else if (stat_diff_player.speed < 0) {
			action_text += "Your speed fell!\n";
		}
		if (stat_diff_player.atk > 0) {
			action_text += "Your attack increased!\n";
		} else if (stat_diff_player.atk < 0) {
			action_text += "Your attack fell!\n";
		}
		if (stat_diff_player.def > 0) {
			action_text += "Your defence increased!\n";
		} else if (stat_diff_player.def < 0) {
			action_text += "Your defence fell!\n";
		}
	}
	if (stat_diff_enemy.hp == 0) {
		if (stat_diff_enemy.speed > 0) {
			action_text += Enemy->name + "'s speed increased!\n";
		} else if (stat_diff_enemy.speed < 0) {
			action_text += Enemy->name + "'s speed fell!\n";
		}
		if (stat_diff_enemy.atk > 0) {
			action_text += Enemy->name + "'s attack increased!\n";
		} else if (stat_diff_enemy.atk < 0) {
			action_text += Enemy->name + "'s attack fell!\n";
		}
		if (stat_diff_enemy.def > 0) {
			action_text += Enemy->name + "'s defence increased!\n";
		} else if (stat_diff_enemy.def < 0) {
			action_text += Enemy->name + "'s defence fell!\n";
		}
	}
	return action_text;
}

int BattleHandler::check_end() {
	if (Player->stats.hp <= 0) {
		return -1;
	} else if (Enemy->stats.hp <= 0) {
		return 1;
	} else {
		return 0;
	}
}

void BattleHandler::generate_px_actions(shared_ptr<Fighter> px, int x) {
	shared_ptr<BattleHandler::state_t> state = make_shared<BattleHandler::state_t>();
	state->fighter_atk = px;
	state->fighter_def = px == Player ? Enemy : Player;
	if (x == 0) {
		state->animation_id = att_result->attacks.first.attack_name;
	} else {
		state->animation_id = att_result->attacks.second.attack_name;
	}
	state->text_to_display = generate_display_action_text(px == Player);
	BattleHandler::states.push(state);
	state = make_shared<BattleHandler::state_t>();
	state->fighter_atk = px;
	state->fighter_def = px == Player ? Enemy : Player;
	state->animation_id = "state:healthbar_anim"; // Lebensbalkenanimation. Falls nicht notwendig wird das auch nicht gemacht in der GUI. Braucht aber immer konstante Zeit, damit der text gelesen werden kann.
	state->text_to_display = generate_display_stats_change_text(px == Player);
	BattleHandler::states.push(state);
}

void BattleHandler::generate_check_end() {
	shared_ptr<BattleHandler::state_t> state = make_shared<BattleHandler::state_t>();
	state->animation_id = "state:checkend"; // bedeutet: Schau mal bitte ob alle noch leben und wenn nicht beende das Spiel
	BattleHandler::states.push(state);
}

void BattleHandler::generate_ask_okay() {
	shared_ptr<BattleHandler::state_t> state = make_shared<BattleHandler::state_t>();
	state->animation_id = "state:ask_for_okay"; // bedeutet: Frag nach nem okay (GUI)
	BattleHandler::states.push(state);
}

void BattleHandler::attack_result_to_states() {
	shared_ptr<Fighter> p1 = BattleHandler::att_result->p1_is_player ? Player : Enemy;
	shared_ptr<Fighter> p2 = !BattleHandler::att_result->p1_is_player ? Player : Enemy;

	// p1 actions
	generate_px_actions(p1, 0);

	// checkEnd generate Win/Lose state
	generate_check_end();

	// ask okay
	generate_ask_okay();

	// p2 actions
	generate_px_actions(p2, 1);

	// checkEnd generate Win/Lose state
	generate_check_end();

	// ask okay
	generate_ask_okay();

	// I believe that's it
	cout << "Player: hp: " << Player->stats.hp << ", atk: " << Player->stats.atk << ", def: " << Player->stats.def << ", speed: " << Player->stats.speed << endl;
	cout << "Enemy: hp: " << Enemy->stats.hp << ", atk: " << Enemy->stats.atk << ", def: " << Enemy->stats.def << ", speed: " << Enemy->stats.speed << endl;
}

void add_texture(vector<shared_ptr<Drawelement>> prototype, string tex, string path) {
	for (const auto &elem : prototype) {
		elem->material->add_texture("specular", make_texture(tex, path));
	}
}

void BattleHandler::init_statics() {
	if (!first_call) return;

	// load the arena
	prototype_arena = MeshLoader::load_animated("render-data/world/arena.dae", "idle", glm::translate(glm::vec3(5, 0, 0)));
	update_loading_screen();

	// Set Player and Enemy and their trafos
	// Load Player prototype
	auto prototype_player = MeshLoader::load_animated("render-data/player/player_idle.dae", "idle", glm::mat4(1));
	update_loading_screen();
	add_texture(prototype_player, "player_spec", "render-data/player/player_spec.png");
	update_loading_screen();
	MeshLoader::load_animation("render-data/player/player_default_attack.dae", "HotFix", prototype_player, true);
	update_loading_screen();
	MeshLoader::load_animation("render-data/player/player_damage.dae", "damage", prototype_player);
	update_loading_screen();
	MeshLoader::load_animation("render-data/player/player_calc_norms.dae", "Calculate Normals", prototype_player);
	update_loading_screen();
	MeshLoader::load_animation("render-data/player/player_calc_norms.dae", "cout-Debugging", prototype_player);
	update_loading_screen();
	MeshLoader::load_animation("render-data/player/player_cry.dae", "extra_damage", prototype_player);
	update_loading_screen();

	// Load enemy prototypes
	// Load padding prototype
	auto prototype_padding = MeshLoader::load_animated("render-data/padding/padding_idle.dae", "idle", glm::mat4(1));
	update_loading_screen();
	add_texture(prototype_padding, "padding_spec", "render-data/padding/padding_spec.png");
	update_loading_screen();
	MeshLoader::load_animation("render-data/padding/padding_arm_smash.dae", "Arm Smash", prototype_padding, true);
	update_loading_screen();
	MeshLoader::load_animation("render-data/padding/padding_defend.dae", "Defensive Stance", prototype_padding);
	update_loading_screen();
	MeshLoader::load_animation("render-data/padding/padding_body_slam.dae", "Body Slam", prototype_padding);
	update_loading_screen();
	MeshLoader::load_animation("render-data/padding/padding_damage.dae", "damage", prototype_padding);
	update_loading_screen();

	// Load sl prototype
	auto prototype_sl = MeshLoader::load_animated("render-data/sl/sl_idle.dae", "idle", glm::mat4(1));
	update_loading_screen();
	add_texture(prototype_sl, "sl_spec", "render-data/sl/sl_spec.png");
	update_loading_screen();
	MeshLoader::load_animation("render-data/sl/sl_plow.dae", "Plow", prototype_sl, true);
	update_loading_screen();
	MeshLoader::load_animation("render-data/sl/sl_term_vel.dae", "Terminal Velocity", prototype_sl);
	update_loading_screen();
	MeshLoader::load_animation("render-data/sl/sl_smoke_bomb.dae", "Smoke Bomb", prototype_sl);
	update_loading_screen();
	MeshLoader::load_animation("render-data/sl/sl_damage.dae", "damage", prototype_sl);
	update_loading_screen();


	// Load z-Buffer-Fighting
	auto prototype_zbf = MeshLoader::load_animated("render-data/z-buffer-fighting/zbf_idle.dae", "idle", glm::mat4(1));
	update_loading_screen();
	MeshLoader::load_animation("render-data/z-buffer-fighting/zbf_fight_me_2.dae", "Fight Me", prototype_zbf, true);
	update_loading_screen();
	MeshLoader::load_animation("render-data/z-buffer-fighting/zbf_z_buffs.dae", "Z-Buffs", prototype_zbf);
	update_loading_screen();
	MeshLoader::load_animation("render-data/z-buffer-fighting/zbf_wall_of_ugly.dae", "Wall of Ugly", prototype_zbf);
	update_loading_screen();
	MeshLoader::load_animation("render-data/z-buffer-fighting/zbf_damage.dae", "damage", prototype_zbf);
	update_loading_screen();
	MeshLoader::load_animation("render-data/z-buffer-fighting/zbf_extra_damage.dae", "extra_damage", prototype_zbf);
	update_loading_screen();
	auto zbf_shader = make_shader("zbf_shader", "shader/bone.vs", "shader/zbf.fs");
	update_loading_screen();
	for (auto &elem: prototype_zbf)
		elem->use_shader(zbf_shader);


	auto trafo = glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0));
	Player = make_shared<Fighter>(trafo, prototype_player);
	Player->moves = std_moves;
	Player->future_moves = std_future_moves;
	Player->set_base_stats(70, 80, 70, 70);
	Player->level_up(1);
	update_loading_screen();

	glm::vec3 enemy_pos = glm::vec3(10, 0, 0);
	trafo = glm::translate(enemy_pos) * glm::rotate(glm::radians(-90.0f), glm::vec3(0, 1, 0));

	auto padding_monster = make_shared<Fighter>(trafo, prototype_padding);
	padding_monster->name = "Padding";
	padding_monster->moves.push_back("Arm Smash");
	padding_monster->future_moves.push_back("Body Slam");
	padding_monster->future_moves.push_back("Defensive Stance");
	padding_monster->set_base_stats(70, 75, 95, 55);
	padding_monster->type = Fighter::CPU_t;
	update_loading_screen();

	auto sl_monster = make_shared<Fighter>(trafo, prototype_sl);
	sl_monster->name = "sl";
	sl_monster->moves.push_back("Plow");
	sl_monster->future_moves.push_back("Terminal Velocity");
	sl_monster->future_moves.push_back("Smoke Bomb");
	sl_monster->set_base_stats(50, 100, 50, 80);
	sl_monster->type = Fighter::CPU_t;
	update_loading_screen();

	auto zbf_monster = make_shared<Fighter>(trafo, prototype_zbf);
	zbf_monster->name = "Z-Buffer-Fighting";
	zbf_monster->moves.push_back("Fight Me");
	zbf_monster->future_moves.push_back("Z-Buffs");
	zbf_monster->future_moves.push_back("Wall of Ugly");
	zbf_monster->set_base_stats(45, 70, 45, 100);
	zbf_monster->type = Fighter::GPU_t;
	update_loading_screen();

	all_enemies.push_back(padding_monster);
	all_enemies.push_back(sl_monster);
	all_enemies.push_back(zbf_monster);
	update_loading_screen();


	first_call = false;
}

void BattleHandler::init_battle() {
	i_wanna_end_me = false;
	GUI::reset_percentage();
	Moves::init_moves();
	Effects::init_effects();

	Enemy = make_shared<Fighter>(*all_enemies[current_file->enemy_id]);
	Enemy->level_up(current_file->size);
//	Enemy->name += " (" + to_string(Enemy->level) + "kb)";
	if (super_easy) {
		Enemy->level_up(1);
	}
	Enemy->stats = Enemy->level_stats;
	Enemy->max_hp = Enemy->stats.hp;

	Player->level_up(max(1, current_save->data->playerlevel));
	Player->last_move = "";
	Player->name = current_save->data->playername;
	if (super_easy) {
		Player->level_up(100);
		cout << "player level" << Player->level << endl;
	}
	Player->stats = Player->level_stats;
	Player->max_hp = Player->stats.hp;
	GUI::set_bar_char_names({Player->name, Enemy->name});
}

void BattleHandler::battle() { // Enemy bestimmt durch current_enemy
	init_battle();
	new_in_state = true;
	auto battlecam = Camera::find("battlecam");
	battlecam->make_current();
	Jukebox::restart_static_sound(Jukebox::sound::battle_music);

	float vfx_timer = 0;
	bool use_sprite_normal = false;
	bool use_sprite_hot_fix = false;
	bool use_sprite_loading = false;
	Player->do_animation("idle");
	Enemy->do_animation("idle");
	Effects::play_effect("idle");
	while (Context::running() && vfx_timer < 5000) {
		if (Camera::current()->name != "battlecam")
			Camera::default_input_handler(Context::frame_time());
		Camera::current()->update();
		static uint32_t counter = 0;
		if (counter++ % 100 == 0) Shader::reload();
		if (debug_gui) {
			ImGui::Begin("Load Time");
			ImGui::SliderFloat("vfx_timer", &vfx_timer, -10000, 5000);
			ImGui::End();
		} else {
			vfx_timer += Context::frame_time();
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		the_skybox->draw(sky_shader);
		text_screen->use_texts_input({"You found a " + all_enemies[current_file->enemy_id]->name + "-Bug while commiting!"});
		text_screen->draw();
		arena_draw(arena_loading_shader, vfx_timer);
		Player->draw(sprite_loading_shader, vfx_timer, true);
		Enemy->draw(sprite_loading_shader, vfx_timer, true);
		Context::swap_buffers();
	}

	for (const auto &elem : prototype_arena) {
		elem->do_animation("idle");
		if (elem->name.find("Circle") != string::npos) {
			elem->use_shader(arena_floor_shader);
		} else {
			elem->use_shader(arena_wall_shader);
		}
	}

	while (Context::running()) {
		// cout << "states size: " << states.size() << endl;
		if (Camera::current()->name != "battlecam")
			Camera::default_input_handler(Context::frame_time());
		Camera::current()->update();
		static uint32_t counter = 0;
		if (counter++ % 100 == 0) Shader::reload();
		if (debug_gui) {
			ImGui::Begin("choose animation");
			static int anim_id = 0;
			ImGui::SliderInt("anim_id", &anim_id, 0, 1);
			ImGui::End();
		}
		vfx_timer += Context::frame_time();

		// render
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (new_in_state) {
			Player->do_animation("idle");
			Enemy->do_animation("idle");
			Effects::play_effect("idle");
			use_sprite_normal = false;
			use_sprite_hot_fix = false;
			use_sprite_loading = false;
		}
		if (states.empty()) { // wait for attack input
			if (i_wanna_end_me) {
				Jukebox::pause_static_sound(Jukebox::sound::battle_music);
				overlay_screen->update_overlay();
				Player->level_up(max(1, current_save->data->playerlevel));
				if (Files::commited.front() == current_file) current_save->save_me(true);
				return;
			}
			if (new_in_state) {
				vector<string> attacks;
				for (auto &attack_id : Player->moves) {
					string this_move = Moves::all_moves[attack_id].name;
					// cout << "This move " << attack_id << " is: " << this_move << endl;
					if (this_move != Player->last_move) attacks.push_back(this_move);
//					attacks.push_back(this_move);
				}
				cout << "There are " << attacks.size() << " attacks" << endl;
				fight_screen->use_texts_input(attacks, true);
				fight_screen->set_scroll_page(0);
				new_in_state = false;
			}
			// kann man vielleicht noch aus dem if raus schieben
			// cout << "drawing_fight_screen" << endl;
			fight_screen->draw();
			// cout << "get clicked bool" << endl;
			string clicked_attack = fight_screen->get_clicked_text_id();
			if (clicked_attack != "no_attack") { // actually clicked something
				// cout << "got clicked " << Player->moves.size() << endl;
				new_in_state = true;
				Player->last_move = clicked_attack;
				att_result = calc_attack_results(clicked_attack);
				// cout << "made click calculations" << endl;
				attack_result_to_states();
				// cout << "added click states" << endl;
			}
		} else { // there are states to be done
			shared_ptr<state_t> current_state = states.front();
			if (debug_gui) {
				cout << "This is the current state: " << endl;
				cout << (current_state->fighter_atk == Player ? "This is the Player." : "This is the Enemy") << endl;
				cout << "The Animation id is " << current_state->animation_id << endl;
				cout << "The Text is: " << current_state->text_to_display << endl << endl << endl;
			}
			if (current_state->animation_id == "state:checkend") {
				if (new_in_state) {
					new_in_state = false;
				}
				int end = check_end();
				if (end == -1) {
					prev_text_to_display = "You weren't able to fix the bug in time for the final commit. You have been exmatriculated...";
					Player->do_animation("extra_damage");
					game_mode = "start_screen";
				} else if (end == 1) {
					prev_text_to_display = "Congratulations! You have fixed the bug!";
					Files::collected.remove(current_file);
					Files::commited.push_front(current_file);
					current_save->apply_current_files();
					game_mode = "world3d";
					if(Files::files.empty() && Files::collected.empty()) {
						won = true;
						game_mode = "victory_screen";
					}
				}
				if (end) {
					i_wanna_end_me = true;
					while (states.size() > 1) {
						states.pop();
					}
				} else {
					states.pop();
				}
				continue;
			} else if (current_state->animation_id == "state:ask_for_okay") { // ask_for_okay GUI
				if (new_in_state) {
					new_in_state = false;
				}
				text_and_okay_screen->use_texts_input({prev_text_to_display});
				text_and_okay_screen->draw();
				if ("no_attack" != text_and_okay_screen->get_clicked_text_id()) {
					states.pop();
					new_in_state = true;
				}

			} else if (current_state->animation_id == "state:healthbar_anim") { // Lebensbalken GUI
				int pl = current_state->fighter_atk == Player ? 0 : 1;
				if (new_in_state) {
					auto f_atk = current_state->fighter_atk;
					auto f_def = current_state->fighter_def;
					if ((att_result->p1_is_player && pl == 0) || (!att_result->p1_is_player && pl == 1)) {
						f_atk->stats.hp = glm::clamp(f_atk->stats.hp + att_result->attacks.first.stat_change.first.hp, 0, f_atk->max_hp);
						f_def->stats.hp = glm::clamp(f_def->stats.hp + att_result->attacks.first.stat_change.second.hp, 0, f_def->max_hp);
					} else {
						f_atk->stats.hp = glm::clamp(f_atk->stats.hp + att_result->attacks.second.stat_change.first.hp, 0, f_atk->max_hp);
						f_def->stats.hp = glm::clamp(f_def->stats.hp + att_result->attacks.second.stat_change.second.hp, 0, f_def->max_hp);
					}
					cout << current_state->fighter_atk->name << ": " << current_state->fighter_atk->stats.hp << "hp" << endl
						 << current_state->fighter_def->name << ": " << current_state->fighter_def->stats.hp << "hp" << endl;
					GUI::set_percentage(pl, (float) current_state->fighter_atk->stats.hp / (float) current_state->fighter_atk->max_hp);
					GUI::set_percentage(1 - pl, (float) current_state->fighter_def->stats.hp / (float) current_state->fighter_def->max_hp);
				}
				bool keep_on = text_screen->draw_healthbar_anim(new_in_state, pl);
				keep_on |= text_screen->draw_healthbar_anim(new_in_state, 1 - pl);
				text_screen->use_texts_input({"" == current_state->text_to_display ? prev_text_to_display : current_state->text_to_display});
				prev_text_to_display = "" == current_state->text_to_display ? prev_text_to_display : current_state->text_to_display;
				text_screen->draw();
				if (new_in_state) {
					new_in_state = false;
				}
				if (!keep_on) { states.pop(); }
			} else {
				if (new_in_state) {
					int pl = current_state->fighter_atk == Player ? 0 : 1;
					if ((att_result->p1_is_player && pl == 0) || (!att_result->p1_is_player && pl == 1)) {
						string fighter_atk_name = current_state->fighter_atk == Player ? "Player" : current_state->fighter_atk->name;
						string fighter_def_name = current_state->fighter_def == Player ? "Player" : current_state->fighter_def->name;
						if (att_result->attacks.first.stat_change.first.hp < 0) {
							current_state->fighter_atk->do_animation("damage");
							Jukebox::play_sound_string(fighter_def_name + "_damage");
						} else {
							current_state->fighter_atk->do_animation(current_state->animation_id);
						}
						if (att_result->attacks.first.stat_change.second.hp < 0) {
							if (att_result->attacks.first.attack_name == "glPolygonOffset") {
								current_state->fighter_def->do_animation("extra_damage");
								Jukebox::play_sound_string(fighter_def_name + "_extra_damage");
							} else {
								current_state->fighter_def->do_animation("damage");
								Jukebox::play_sound_string(fighter_def_name + "_damage");
							}
							if (att_result->attacks.first.attack_name == "Calculate Normals") {
								use_sprite_normal = true;
							} else if (att_result->attacks.first.attack_name == "HotFix") {
								use_sprite_hot_fix = true;
							} else if (att_result->attacks.first.attack_name == "Off and On again") {
								use_sprite_loading = true;
							}
						}
						Jukebox::play_sound_string(fighter_atk_name + "_" + att_result->attacks.first.attack_name);
					} else {
						string fighter_atk_name = current_state->fighter_atk == Player ? "Player" : current_state->fighter_atk->name;
						string fighter_def_name = current_state->fighter_def == Player ? "Player" : current_state->fighter_def->name;
						if (att_result->attacks.second.stat_change.first.hp < 0) {
							current_state->fighter_atk->do_animation("damage");
							Jukebox::play_sound_string(fighter_def_name + "_damage");
						} else {
							current_state->fighter_atk->do_animation(current_state->animation_id);
						}
						if (att_result->attacks.second.stat_change.second.hp < 0) {
							if (att_result->attacks.second.attack_name == "glPolygonOffset") {
								current_state->fighter_def->do_animation("extra_damage");
								Jukebox::play_sound_string(fighter_def_name + "_extra_damage");
							} else {
								current_state->fighter_def->do_animation("damage");
								Jukebox::play_sound_string(fighter_def_name + "_damage");
							}
							if (att_result->attacks.second.attack_name == "Calculate Normals") {
								use_sprite_normal = true;
							} else if (att_result->attacks.second.attack_name == "HotFix") {
								use_sprite_hot_fix = true;
							}
						}
						Jukebox::play_sound_string(fighter_atk_name + "_" + att_result->attacks.second.attack_name);
					}
//					Effects::play_effect("HP++", current_state->fighter_atk->trafo);
					Effects::play_effect(current_state->animation_id, current_state->fighter_atk->trafo);
					Effects::time_hot_fix = 0;
					vfx_timer = 0;
					new_in_state = false;
				}
				text_screen->use_texts_input({current_state->text_to_display});
				prev_text_to_display = current_state->text_to_display;
				text_screen->draw();
			}
		}
		if (!bone_shader) cout << "NO SHADER" << endl;
		shared_ptr<Shader> enemy_shader = sprite_shader;
		if (use_sprite_normal) {
			enemy_shader = sprite_normal_shader;
		} else if (use_sprite_hot_fix) {
			enemy_shader = sprite_hot_fix_shader;
		}
		the_skybox->draw(sky_shader);
		arena_draw();
		health_bar_screen->draw();
		Player->draw(sprite_shader);

		if (use_sprite_loading) {
			enemy_shader = sprite_loading_shader;
			Enemy->draw(enemy_shader, abs(vfx_timer * -5 + 5000), true);
		} else {
			Enemy->draw(enemy_shader, vfx_timer, false);
		}
		Effects::draw(bone_shader);
		Context::swap_buffers();
	}
	cout << "stopped running" << endl;
}

void arena_draw(const std::shared_ptr<Shader> &shader, float time) {
	for (const auto &elem : BattleHandler::prototype_arena) {
		elem->update();
		if (shader) {
			elem->use_shader(shader);
		}
		elem->bind();
		if (time != -1) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			elem->shader->uniform("wire", true);
			if (elem->name.find("Circle") != string::npos) {
				elem->shader->uniform("upper_limit", (time - 2500) / 2000.0f);
				elem->shader->uniform("floor", true);
			} else {
				elem->shader->uniform("upper_limit", (time - 2000.0f) / 200.0f);
				elem->shader->uniform("floor", false);
			}
			elem->draw(glm::mat4(1));
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			elem->shader->uniform("wire", false);
			if (elem->name.find("Circle") != string::npos) {
				elem->shader->uniform("upper_limit", (time - 2500) / 2000.0f);
				elem->shader->uniform("floor", true);
			} else {
				elem->shader->uniform("upper_limit", (time - 2000.0f) / 200.0f);
				elem->shader->uniform("floor", false);
			}
		}
		setup_light(elem->shader);
		elem->draw(glm::mat4(1));
		elem->unbind();
	}
}
