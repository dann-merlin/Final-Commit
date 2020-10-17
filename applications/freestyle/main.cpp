#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext.hpp>
#include <cppgl/framebuffer.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "btBulletDynamicsCommon.h"
#include "gui.h"
#include <cppgl/cppgl.h>
#include <imgui/imgui.h>
#include "jukebox.h"
#include "rendering.h"
#include "cppgl/animation/animation.h"
#include "skybox.h"
#include "world3dhandler.h"
#include "battle_system/battlehandler.h"
#include "battle_system/effects.h"

#define MAX_NUM_ACTOR_SHAPES 128

using namespace std;
using namespace glm;
using namespace Jukebox;

// ---------------------------------------
// globals

shared_ptr<World3DHandler> world3dhandler;
shared_ptr<World> world;
shared_ptr<Skybox> the_skybox;
shared_ptr<Player> player_object;
shared_ptr<Framebuffer> gbuffer, blur_buffer;
shared_ptr<save_t> current_save;
shared_ptr<Physic> physic;
shared_ptr<Files::file> current_file, current_enemy;
btVector3 player_dir;
bool stop = true;
bool was_moving = false;
bool debug_gui = false;
bool super_easy = false;
float global_time = 0;

vector<string> std_moves = {"cout-Debugging", "Calculate Normals", "Off and On again"};
vector<string> std_future_moves = {"HP++", "glPolygonOffset", "HotFix"};

int background_flags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMouseInputs;
string start_screen_str = "start_screen", battle_str = "battle", world3d_str = "world3d";
string game_mode = start_screen_str;
string next_enemy = "the baaaad guy";
std::shared_ptr<Screen> active_screen, start_screen, fight_screen, text_and_okay_screen, health_bar_screen, text_screen, victory_screen,
		overlay_screen, checkpoint_screen;
shared_ptr<BattleHandler> battlehandler;
shared_ptr<Player3D> the_player3d;
bool won = false;
ivec2 resolution;

shared_ptr<Shader> world_shader, world3d_dpn_shader, world_sky_shader, sky_shader, rope_shader, checkpoint_shader, world_floor_shader;
shared_ptr<Shader> hot_fix_shader, game_over_screen_shader, matrix_shader, victory_shader;
shared_ptr<Shader> bone_shader, sprite_shader, sprite_loading_shader, sprite_normal_shader, sprite_hot_fix_shader, sprite_static_shader;
shared_ptr<Shader> arena_wall_shader, arena_floor_shader, arena_loading_shader;

// ---------------------------------------
// callbacks

void keyboard_callback(int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
		Camera::find("default")->make_current();
		glfwSetInputMode(Context::instance().glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
		Camera::find("playercam")->make_current();
		glfwSetInputMode(Context::instance().glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	if (key == GLFW_KEY_F4 && action == GLFW_PRESS) debug_gui = !debug_gui;
	if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
		static bool wireframe = false;
		wireframe = !wireframe;
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
	}
	if (key == GLFW_KEY_F6 && action == GLFW_PRESS) {
		if (glIsEnabled(GL_CULL_FACE))
			glDisable(GL_CULL_FACE);
		else
			glEnable(GL_CULL_FACE);
	}
	// HINT: https://www.glfw.org/docs/latest/input_guide.html
	if (action == GLFW_REPEAT) return;
//	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
//		player_object->player_rigid->activate();
//		player_object->player_rigid->applyCentralImpulse(player_dir);
//		was_moving = true;
//	}
//	if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
//		if (was_moving) {
//			player_object->player_rigid->activate();
//			player_object->player_rigid->applyCentralImpulse(-player_dir);
//			was_moving = false;
//		}
//	}
	if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
		if (!super_easy) {
			cout << "0 skill mode activated" << endl;
			super_easy = true;
		} else {
			super_easy = false;
		}
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		Context::time_pass = abs(Context::time_pass - 1);
	}
	if (key == GLFW_KEY_V && action == GLFW_PRESS) {
		won = true;
	}
//	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
//		cout << "Files:\n";
//		for (auto &f : Files::files) {
//			cout << f->id << endl;
//		}
//		cout << "Collected:\n";
//		for (auto &f : Files::collected) {
//			cout << f->id << endl;
//		}
//		cout << "Committed:\n";
//		for (auto &f : Files::commited) {
//			cout << f->id << endl;
//		}
//		cout << endl;
//	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		stop = !stop;
	}
	if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		if (game_mode == world3d_str) {
			bool currently = world3dhandler->get_interactive_mode();
			if (!currently) {
				world3dhandler->try_to_activate_checkpoint_overlay();
			} else {
				world3dhandler->deactivate_checkpoint_overlay();
			}
		}
		// current_save->save_me();
	}
	if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		toggle_mute();
	}
}

void resize_callback(int w, int h) {
	Context::resize(w, h);
	gbuffer->resize(w, h);
	blur_buffer->resize(w, h);
	resolution = ivec2(w, h);
}

void init_shader() {
	update_loading_screen();
	victory_shader = make_shader("victory_shader", "shader/copytex.vs", "shader/victory.fs");
	update_loading_screen();
	game_over_screen_shader = make_shader("game_over_screen_shader", "shader/copytex.vs", "shader/game_over.fs");
	update_loading_screen();
	world_sky_shader = make_shader("world_sky_shader", "shader/sky.vs", "shader/world_sky.fs");
	update_loading_screen();
	bone_shader = make_shader("bone_shader", "shader/bone.vs", "shader/bone.fs");
	update_loading_screen();
	sky_shader = make_shader("sky_shader", "shader/sky.vs", "shader/matrixsky.fs");
	update_loading_screen();
	hot_fix_shader = make_shader("hot_fix_shader", "shader/copytex.vs", "shader/hot_fix.fs");
	update_loading_screen();
	// sky_shader = make_shader("sky_shader", "shader/sky.vs", "shader/sky.fs");
	update_loading_screen();
	rope_shader = make_shader("rope_shader", "shader/rope.vs"/*, "shader/rope.gs"*/, "shader/rope.fs");
	update_loading_screen();
	world3d_dpn_shader = make_shader("world3d_dpn_shader", "shader/world3d_dpn.vs", "shader/world3d_dpn.fs");
	update_loading_screen();
	world_shader = make_shader("world_shader", "shader/world.vs", "shader/world.fs");
	update_loading_screen();
	world_floor_shader = make_shader("world_floor_shader", "shader/world3d_dpn.vs", "shader/arena_floor.fs");
	update_loading_screen();
	checkpoint_shader = make_shader("checkpoint_shader", "shader/checkpoint.vs", "shader/checkpoint.fs");
	update_loading_screen();
	update_loading_screen();
	matrix_shader = make_shader("matrix_shader", "shader/copytex.vs", "shader/matrix.fs");
	update_loading_screen();
	sprite_loading_shader = make_shader("sprite_loading_shader", "shader/sprite_loading.vs", "shader/sprite_loading.fs");
	update_loading_screen();
	sprite_normal_shader = make_shader("sprite_normal_shader", "shader/bone.vs", "shader/sprite_normal.fs");
	update_loading_screen();
	sprite_hot_fix_shader = make_shader("sprite_hot_fix_shader", "shader/bone.vs", "shader/sprite_hot_fix.fs");
	update_loading_screen();
	sprite_shader = make_shader("sprite_shader", "shader/bone.vs", "shader/sprite.fs");
	update_loading_screen();
	sprite_static_shader = make_shader("sprite_static_shader", "shader/sprite.vs", "shader/sprite_static.fs");
	update_loading_screen();
	arena_loading_shader = make_shader("arena_loading_shader", "shader/sprite_loading.vs", "shader/arena_loading.fs");
	update_loading_screen();
	arena_wall_shader = make_shader("arena_wall_shader", "shader/bone.vs", "shader/arena_wall.fs");
	update_loading_screen();
	arena_floor_shader = make_shader("arena_floor_shader", "shader/bone.vs", "shader/arena_floor.fs");
	update_loading_screen();
}

// ---------------------------------------
// main

int main(int argc, char **argv) {
	std::time_t t = std::time(0);
	std::tm *now = std::localtime(&t);
	srand(now->tm_sec + now->tm_min + now->tm_hour);

	// init context and set parameters
	init_jukebox();
	ContextParameters params;
	params.title = "Final Commit";
	params.font_ttf_filename = concat(EXECUTABLE_DIR, "render-data/fonts/DroidSansMono.ttf");
	params.font_size_pixels = 16;
	Context::init(params);
	Context::set_keyboard_callback(keyboard_callback);

	const ivec2 res = Context::resolution();
	gbuffer = make_framebuffer("gbuffer", res.x, res.y);
	gbuffer->attach_depthbuffer(make_texture("gbuf_depth", res.x, res.y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT));
	gbuffer->attach_colorbuffer(make_texture("gbuf_diff", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT));
	gbuffer->attach_colorbuffer(make_texture("gbuf_pos", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT));
	gbuffer->attach_colorbuffer(make_texture("gbuf_norm", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT));
	gbuffer->check();
	blur_buffer = make_framebuffer("blur_buffer", res.x, res.y);
	blur_buffer->attach_depthbuffer(make_texture("blur_buf_depth", res.x, res.y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT));
	blur_buffer->attach_colorbuffer(make_texture("blur_buf_tex", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT));
	blur_buffer->check();

	Context::set_resize_callback(resize_callback);
	glClearColor(0, 0, 0, 1);
	update_loading_screen();
	init_shader();

	// EXECUTABLE_DIR set via cmake, paths now relative to source/executable directory
	Shader::base_path = EXECUTABLE_DIR;
	Texture2D::base_path = EXECUTABLE_DIR;
	MeshLoader::base_path = EXECUTABLE_DIR;

	// init screens

	fight_screen = make_shared<Screen>("fight");
	text_and_okay_screen = make_shared<Screen>("text_and_okay");
	text_screen = make_shared<Screen>("text");
	start_screen = make_shared<Screen>("start");
	victory_screen = make_shared<Screen>("victory");
	health_bar_screen = make_shared<Screen>("health_bar");
	overlay_screen = make_shared<Screen>("overlay");
	checkpoint_screen = make_shared<Screen>("checkpoint");
	active_screen = start_screen;
	update_loading_screen();

	// init stuff

	the_skybox = make_shared<Skybox>();
	auto battlecam = make_camera("battlecam");
	battlecam->far = 20000;
//	battlecam->pos = glm::vec3(-0.3, 1.4, 2.0);
//	battlecam->dir = glm::vec3(0.78, -0.2, -0.58);
	battlecam->pos = glm::vec3(-0.26, 1.37, 2.68);
	battlecam->dir = glm::vec3(0.687, -0.244, -0.685);
	auto player_cam = make_camera("playercam");
	player_cam->make_current();
	update_loading_screen();

	world3dhandler = make_shared<World3DHandler>();
	battlehandler = make_shared<BattleHandler>();
	update_loading_screen();


	auto cam = Camera::current();
	cam->pos = vec3(0, 4, 2);
	cam->dir = vec3(1, 0, 0);

	Effects::init_effects();
	BattleHandler::init_statics();
	Effects::play_effect("Calculate Normals");
	while (Context::running()) {
		// reload shader
		static uint32_t counter = 0;
		if (counter++ % 100 == 0) Shader::reload();

		glfwSetInputMode(Context::instance().glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (game_mode == "start_screen") {
			draw_shader(matrix_shader);
			// draw_shader(hot_fix_shader);
			start_screen->draw_start_screen();
		} else if (game_mode == "world3d") {
			world3dhandler->start_3D(current_save);
		} else if (game_mode == "battle") {
			battlehandler->battle();
		} else if (game_mode == "victory_screen") {
			victory_screen->draw_victory_screen();
		}
		// cout << "\n\n\n\n\n\nmain buffer swap " << game_mode << "\n\n\n\n\n";
		Context::swap_buffers();
	}

	return 0;
}
