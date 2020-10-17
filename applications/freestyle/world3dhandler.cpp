#include "world3dhandler.h"
#include "gui.h"
#include <glm/gtc/type_ptr.hpp>
#include <cppgl/framebuffer.h>
#include "checkpoints.h"
#include "rendering.h"
#include "jukebox.h"
#include "skybox.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext.hpp>

using namespace std;
using namespace glm;

extern bool stop, won;
extern shared_ptr<Framebuffer> gbuffer;
extern btVector3 player_dir;
extern shared_ptr<Shader> world_sky_shader, bone_shader, world_shader, sky_shader, test_shader, rope_shader, checkpoint_shader, world3d_dpn_shader, matrix_shader, game_over_screen_shader, sprite_static_shader, world_floor_shader;
extern shared_ptr<Player> player_object;
extern shared_ptr<Screen> overlay_screen, checkpoint_screen, victory_screen;
extern shared_ptr<save_t> current_save;
extern shared_ptr<Framebuffer> gbuffer;
extern string game_mode;

extern shared_ptr<Skybox> the_skybox;
extern shared_ptr<Player3D> the_player3d;
shared_ptr<Rope> the_rope;
shared_ptr<Physic> World3DHandler::physics;
shared_ptr<World> World3DHandler::world;
shared_ptr<Texture2D> World3DHandler::the_crosshair;
shared_ptr<Files> World3DHandler::the_files;
shared_ptr<Soup> the_soup;
bool interactive_overlay = false, game_over = false, reboot_needed = false, shutdown_needed = false;
vec3 last_dir;
vec3 current_anchor;
irrklang::ISound *grappling_sound = nullptr;
uint game_over_okay_button;
shared_ptr<Shader> blur_shader;
shared_ptr<Texture2D> game_over_tex;
extern shared_ptr<Framebuffer> blur_buffer;

Camera *cam;

World3DHandler::World3DHandler() {
	physics = make_shared<Physic>();
	player_object = make_shared<Player>();
	world = make_shared<World>();
	the_player3d = make_shared<Player3D>(Physic::dynamics_world);
	the_player3d->add_rigid_body();
	the_rope = make_shared<Rope>();
	the_rope->add_rope(the_player3d->capsule_rb, vec3(0, 10, 0));
	the_crosshair = make_texture("crosshair", "render-data/images/crosshair.png");
	the_files = make_shared<Files>(the_player3d->prototype);
	the_soup = make_shared<Soup>(-200);
	Checkpoints::init_checkpoints();
	game_over_okay_button = GUI::create_button("Back to main menu", glm::vec2(1.f - 0.22f, 1.f - 0.1f), glm::vec2(0.2, 0.08), glm::vec4(1), glm::vec4(1), glm::vec4(1), glm::vec4(1), 2.f, 0.f, GUI::FONT::RETRO, Jukebox::play_beep);
	game_over_tex = make_texture("game_over_tex", "render-data/images/game_over.png");
	blur_shader = make_shader("blur_shader", "shader/copytex.vs", "shader/blur.fs");
}

void World3DHandler::reboot() {
	reboot_needed = true;
}

void World3DHandler::shutdown() {
	shutdown_needed = true;
}

void World3DHandler::load_save(shared_ptr<save_t> save) {
	won = false;
	save->load_me();
	auto cp = Checkpoints::checkpoints[save->data->checkpointid];
	auto spawn_pos = cp->pos + vec3(0,the_player3d->characterHeight * 0.5f + the_player3d->characterWidth, 0);
	// cerr << "spawn pos is: " << glm::to_string(spawn_pos) << " for checkpoint " << cp->id << endl;
	the_player3d->pos = spawn_pos;
	Camera::find("playercam")->pos = spawn_pos;

	the_rope->remove_rope();
	// sets the position for bullet
	btTransform transform = the_player3d->capsule_rb->getCenterOfMassTransform();
	transform.setOrigin(btVector3(spawn_pos.x, spawn_pos.y, spawn_pos.z));
	the_player3d->capsule_rb->setCenterOfMassTransform(transform);
	the_player3d->capsule_rb->setLinearVelocity(btVector3(0.f,0.f,0.f));
}

void World3DHandler::start_3D(shared_ptr<save_t> save) {
	// cout << "starting 3d mode" << endl;
	glfwSetInputMode(Context::instance().glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	Camera::find("playercam")->make_current();
	// Load save
	load_save(save);
	shutdown_needed = false;
	reboot_needed = false;
	game_over = false;
	Jukebox::damp_music(false);
	Jukebox::play_static_sound(Jukebox::sound::wind);

	// LOOP
	while(Context::running()) {
		// ImGui::Begin("FOV Settings");
		// ImGui::DragFloat("FOV", &Camera::current()->fov_degree);
		// ImGui::End();
		if(shutdown_needed) {
			shutdown_needed = false;
			return;
		}
		if(reboot_needed) {
			load_save(current_save);
			reboot_needed = false;
		}

		// reload shader
		static uint32_t counter = 0;
		if (counter++ % 100 == 0) Shader::reload();

		// input handlers
		cam = Camera::current();
		if (cam->name != "playercam") {
			Camera::default_input_handler(Context::frame_time());
		} else {
			Camera::default_mouse_handler(Context::frame_time());
			the_player3d->default_input_handler(Context::frame_time());
		}


		if(!game_over) {
			// draw rope
			if (the_player3d->shoot_rope) generate_rope();
			if (!the_player3d->active_rope) {
				// the_player3d->remove_rigid_body();
				the_rope->remove_rope();
			}
			// update
			update();
		}

		// render
		// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// gbuffer->bind();
		// gbuf_draw();
		// gbuffer->unbind();
		draw();
		if(!interactive_overlay) overlay_screen->draw();
		if(!game_over && interactive_overlay) {
			checkpoint_screen->draw_checkpoint_screen();
		}

		if(game_over) {
			static float t = 0;
			t+= Context::frame_time();
			draw_shader(game_over_screen_shader, t, game_over_tex);
			shared_ptr<GUI::container_t> c = Screen::containers[game_over_okay_button];
			c->draw();
			if(c->isClicked()) {
				shutdown_needed = true;
				game_mode = "start_screen";
			}
		} else if(won) {
			game_mode = "victory_screen";
			shutdown_needed = true;
		}

		// finish / swap buffers
		Context::swap_buffers();
	}
}

bool World3DHandler::get_interactive_mode() {
	return interactive_overlay;
}

void World3DHandler::try_to_activate_checkpoint_overlay() {
	if(!Checkpoints::show_overlay_allowed()) return;
	interactive_overlay = true;
	glfwSetInputMode(Context::instance().glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	Context::time_pass = 0.f;
	Jukebox::damp_music(true);
	Jukebox::pause_static_sound(Jukebox::sound::wind);
	current_save->data->checkpointid = Checkpoints::closest_checkpoint_to_player->id;
	checkpoint_screen->use_texts_input({"Files...", "Save Game", "Load Game", "Back to game", "Back to main menu", "Exit Game"}, true);
	checkpoint_screen->update_checkpoint_screen();
}

void World3DHandler::deactivate_checkpoint_overlay(bool activate_music) {
	interactive_overlay = false;
	glfwSetInputMode(Context::instance().glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	Context::time_pass = 1.f;
	if(activate_music) {Jukebox::damp_music(false); Jukebox::play_static_sound(Jukebox::sound::wind);}
}

void World3DHandler::generate_rope() {
	GLfloat depth[4];
	gbuffer->bind();
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glReadPixels(Context::resolution().x / 2, Context::resolution().y / 2, 1, 1, GL_RGBA, GL_FLOAT, depth);
	gbuffer->unbind();
	vec3 anchor = vec3(depth[0], depth[1], depth[2]);
	if (anchor == vec3(0)) {
		if(the_player3d->jetpack) the_player3d->active_rope = false;
		// the_player3d->active_rope = false;
		// the_player3d->shoot_rope = false;
		// the_player3d->draw_rigid_player = false;
		// the_player3d->switch_to_kinnematic_player = false;
		return;
	}
	the_player3d->t.restart();
	cout << "Anchor pos: " << to_string(anchor) << endl;

	the_rope->remove_rope();
	// the_player3d->add_rigid_body();

	the_rope->add_rope(the_player3d->capsule_rb, anchor);
	current_anchor = anchor;
	Jukebox::stop_sound(grappling_sound);
	grappling_sound = Jukebox::play_sound(Jukebox::sound::grappling);

	the_player3d->shoot_rope = false;
}


void World3DHandler::update() {

	if (stop) Physic::m_dynamics_world->stepSimulation(Context::frame_time() * 0.0015, 10);

	the_player3d->capsule_rb->activate();
	the_player3d->update(Physic::dynamics_world);
	Checkpoints::update();
	cam->update();


	// Physics stuff I (Merlin) don't unterstand and am too lazy to read it through
	player_dir = btVector3(cam->dir.x, cam->dir.y, cam->dir.z) * 100000.0f;
	btTransform player_transform;
	glm::mat4 p;
	player_object->player_rigid->getMotionState()->getWorldTransform(player_transform);
	player_transform.getOpenGLMatrix(glm::value_ptr(p));
	btCollisionObject *obj = Physic::m_dynamics_world->getCollisionObjectArray()[1];
	btRigidBody *body = btRigidBody::upcast(obj);
	btTransform trans;
	glm::mat4 m;
	if (body && body->getMotionState()) {
		body->getMotionState()->getWorldTransform(trans);
		trans.getOpenGLMatrix(glm::value_ptr(m));
		/* cout << "Updating cube trafo: " << glm::to_string(m) << endl; */
		//			testcube->trafo = m;
		//			 cout << "cube pos: " << glm::to_string(m[3]) << endl;
	} else {
		/* trans = obj->getWorldTransform(); */
		/* trans.getOpenGLMatrix(glm::value_ptr(world->trafo)); */
	}

	btCollisionObject *obj_world = Physic::m_dynamics_world->getCollisionObjectArray()[0];
	btRigidBody *body_world = btRigidBody::upcast(obj_world);
	btTransform trans_world;
	if (body_world && body_world->getMotionState()) {
		/* body->getMotionState()->getWorldTransform(trans); */
		/* trans.getOpenGLMatrix(glm::value_ptr(testcube->trafo)); */
	} else {
		trans_world = obj_world->getWorldTransform();
		trans_world.getOpenGLMatrix(glm::value_ptr(world->trafo));
	}
	// Physics update stuff is over

	the_files->update();
	the_soup->update();
	if(the_soup->pos.y - 5.f > Camera::find("playercam")->pos.y) { game_over = true; glfwSetInputMode(Context::instance().glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }
	overlay_screen->update_overlay();
	// if(interactive_overlay) checkpoint_screen->update_checkpoint_screen();
}

void World3DHandler::gbuf_draw() {
	// world->draw(world_shader);
}

void World3DHandler::draw() {
	gbuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	world->draw(world3d_dpn_shader);
	gbuffer->unbind();
	// blur_buffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	deferred_world_pass(gbuffer, the_player3d->pos);
	if (the_player3d->third_person)
		the_player3d->draw(NULL);
	if (the_player3d->draw_rigid_player)
		the_rope->draw(rope_shader);

	the_files->draw(sprite_static_shader);
	the_skybox->draw(sky_shader);
	// the_skybox->draw(world_sky_shader);
	the_soup->update_soup_texture();
	the_soup->draw();
	Checkpoints::draw(checkpoint_shader);
	// blur_buffer->unbind();

	// draw_blur(blur_shader, blur_buffer, the_player3d->velocity);
	blit_alpha(the_crosshair);
}
