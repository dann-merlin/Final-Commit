#include "effects.h"
#include "../rendering.h"
#include <cppgl/meshloader.h>
#include <cppgl/context.h>
#include <iostream>


#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

std::unordered_map<std::string, std::vector<std::shared_ptr<Drawelement>>> Effects::effects;
std::unordered_map<std::string, std::shared_ptr<Shader>> Effects::effect_shader;
std::vector<std::shared_ptr<Drawelement>> Effects::current_effect;
std::shared_ptr<Shader> Effects::current_effect_shader;
glm::mat4 Effects::trafo;
bool Effects::loaded = false;
float Effects::time_hot_fix = 0;

void Effects::init_effects() {
	if (loaded) return;
	vector<shared_ptr<Drawelement>> idle_proto;
	effects["effect_idle"] = idle_proto;
	current_effect = idle_proto;

	//load effects
	// effect_shader["effect_HotFix"] = make_shader("HotFix_shader", "shader/bone.vs", "shader/hppp.fs");
	effect_shader["effect_HotFix"] = make_shader("HotFix_shader", "shader/copytex.vs", "shader/hot_fix.fs");
	update_loading_screen();
	// effects["effect_HotFix"] = MeshLoader::load_animated("render-data/effects/hot_fix.dae", "effect_HotFix", glm::scale(vec3(1)));
	effects["effect_HotFix"] = {};
	effect_shader["effect_HP++"] = make_shader("HP++_shader", "shader/bone.vs", "shader/hppp.fs");
	update_loading_screen();
	effects["effect_HP++"] = MeshLoader::load_animated("render-data/effects/hppp.dae", "effect_HP++", glm::scale(glm::vec3(0.1)));
	update_loading_screen();
	effect_shader["effect_Calculate Normals"] = make_shader("Calculate Normals_shader", "shader/bone.vs", "shader/hppp.fs");
	update_loading_screen();
	effects["effect_Calculate Normals"] = MeshLoader::load_animated("render-data/effects/calc_normals.dae", "effect_Calculate Normals", glm::scale(vec3(1)));
	update_loading_screen();
	effect_shader["effect_Wall of Ugly"] = make_shader("Wall of Ugly_shader", "shader/bone.vs", "shader/hppp.fs");
	update_loading_screen();
	effects["effect_Wall of Ugly"] = MeshLoader::load_animated("render-data/effects/wall_of_ugly.dae", "effect_Wall of Ugly", glm::scale(vec3(1)));
	update_loading_screen();
	effect_shader["effect_cout-Debugging"] = make_shader("cout-Debugging_shader", "shader/bone.vs", "shader/hppp.fs");
	update_loading_screen();
	effects["effect_cout-Debugging"] = MeshLoader::load_animated("render-data/effects/cout_debugging.dae", "effect_cout-Debugging", glm::scale(vec3(1)));
	update_loading_screen();
	effect_shader["effect_Smoke Bomb"] = make_shader("Smoke Bomb_shader", "shader/bone.vs", "shader/hppp.fs");
	update_loading_screen();
	effects["effect_Smoke Bomb"] = MeshLoader::load_animated("render-data/effects/smoke_bomb.dae", "effect_Smoke Bomb", glm::scale(vec3(1)));
	update_loading_screen();
	loaded = true;
}

void Effects::play_effect(std::string effect_name, mat4 trafo_new) {
	if (effects.count("effect_" + effect_name) > 0) {
		current_effect = effects["effect_" + effect_name];
		current_effect_shader = effect_shader["effect_" + effect_name];
	} else {
		cerr << "Effect " << effect_name << " was not found" << endl;
		current_effect = effects["effect_idle"];
	}
	for (auto &prototype : current_effect) {
		prototype->do_animation("effect_" + effect_name);
	}
	trafo = trafo_new;
}

void Effects::draw(const std::shared_ptr<Shader> &shader) {
	for (auto &prototype : current_effect) {
		prototype->update();
		if (shader) {
			prototype->use_shader(shader);
			prototype->use_shader(current_effect_shader);
		} else {
			prototype->use_shader(current_effect_shader);
		}

		prototype->bind();
		prototype->shader->uniform("iResolution", Context::resolution());
		setup_light(prototype->shader);
		prototype->draw(trafo);
		prototype->unbind();
	}

	if (current_effect_shader == effect_shader["effect_HotFix"]) {
		time_hot_fix += Context::frame_time();
		draw_shader(current_effect_shader, time_hot_fix);
//		cerr << "drawing Hotfix shader" << endl;
	}
}
