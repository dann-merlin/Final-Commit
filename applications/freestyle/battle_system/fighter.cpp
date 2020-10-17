#include "fighter.h"
#include <cppgl/meshloader.h>
#include <iostream>
#include "../rendering.h"
#include "../save.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

extern shared_ptr<save_t> current_save;

Fighter::Fighter() {
	trafo = mat4(1);
}

Fighter::Fighter(mat4 t, const vector<shared_ptr<Drawelement>> &proto) {
	prototype = proto;
	trafo = t;
	if (prototype.size() > 0) cout << prototype[0]->meshes.size() << endl;
	if (prototype.size() > 0) cout << "has spec " << prototype[0]->material->has_texture("specular") << endl;

}

void Fighter::draw(std::shared_ptr<Shader> shader, float time, bool wire) {
	for (auto &elem : prototype) {
//		elem->do_animation(anim_id);
		elem->update();
		if (shader)
			elem->use_shader(shader);
		elem->bind();
		if (time != -1) {
			if (wire) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				elem->shader->uniform("wire", true);
				elem->shader->uniform("upper_limit", (time - 500.0f) / 1000.0f);
				elem->shader->uniform("time", time);
				elem->draw(trafo);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			elem->shader->uniform("wire", false);
			elem->shader->uniform("upper_limit", (time - 500.0f) / 1000.0f);
			elem->shader->uniform("time", time);
		}
		setup_light(elem->shader);
		elem->draw(trafo);
		elem->unbind();
	}
}

void Fighter::do_animation(std::string anim_id_p) {
	anim_id = anim_id_p;
	for (auto &elem : prototype) {
		elem->do_animation(anim_id);
	}
}

std::string Fighter::get_random_move() {
	if (moves.size() == 0) {
		cerr << "get_random_move: Enemy had no moves" << endl;
		return "Enemy had no moves";
	}
	string move_id = moves[rand() % moves.size()];
	for (uint i = 0; move_id == last_move && i < 5; i++) {
		move_id = moves[rand() % moves.size()];
	}
	last_move = move_id;
	return move_id;
}

void Fighter::apply_stat_diff(stats_t diff) {
	stats.atk += diff.atk;
	stats.def += diff.def;
	stats.speed += diff.speed;
}

void Fighter::set_base_stats(int hp, int atk, int def, int speed) {
	base_stats.hp = hp;
	base_stats.atk = atk;
	base_stats.def = def;
	base_stats.speed = speed;
}

void Fighter::calculate_stats() {
	level_stats.hp = (base_stats.hp + 15.0) * 2.0 * level / 100.0 + level + 10;
	level_stats.atk = (base_stats.atk + 15.0) * 2.0 * level / 100.0 + 5;
	level_stats.def = (base_stats.def + 15.0) * 2.0 * level / 100.0 + 5;
	level_stats.speed = (base_stats.speed + 15.0) * 2.0 * level / 100.0 + 5;
}

void Fighter::level_up(int new_level) {
	int barrier_1 = 25;
	int barrier_2 = 50;
	int barrier_3 = 75;

	if (level < barrier_1 && new_level >= barrier_1 && future_moves.size() > 0) {
		int m_id = rand() % future_moves.size();
		string move_str = future_moves[m_id];
		future_moves.erase(future_moves.begin() + m_id);
		moves.push_back(move_str);
		cout << "Congratulations! " << name << " learned the move " << move_str << endl;
	}
	if (level < barrier_2 && new_level >= barrier_2 && future_moves.size() > 0) {
		int m_id = rand() % future_moves.size();
		string move_str = future_moves[m_id];
		future_moves.erase(future_moves.begin() + m_id);
		moves.push_back(move_str);
		cout << "Congratulations! " << name << " learned the move " << move_str << endl;
	}
	if (level < barrier_3 && new_level >= barrier_3 && future_moves.size() > 0) {
		int m_id = rand() % future_moves.size();
		string move_str = future_moves[m_id];
		future_moves.erase(future_moves.begin() + m_id);
		moves.push_back(move_str);
		cout << "Congratulations! " << name << " learned the move " << move_str << endl;
	}

	if(current_save) current_save->data->playermoves = moves;
	level = new_level;
	calculate_stats();
}
