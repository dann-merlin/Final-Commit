#include <iostream>
#include <imgui/imgui.h>
#include "checkpoints.h"
#include "rendering.h"
#include "world3dhandler.h"
#include "player3d.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

std::vector<std::shared_ptr<Checkpoints::checkpoint>> Checkpoints::checkpoints;
std::shared_ptr<Checkpoints::checkpoint> Checkpoints::closest_checkpoint_to_player;
std::shared_ptr<Drawelement> Checkpoints::prototype;
std::vector<glm::vec3> Checkpoints::verts;
std::vector<unsigned int> Checkpoints::indices;
uint Checkpoints::ids = 0;
float checkpoint_distance_threshold = 5;
extern std::shared_ptr<Player3D> the_player3d;

extern bool debug_gui;

using namespace std;
using namespace glm;

Checkpoints::Checkpoints() {

}

void Checkpoints::init_checkpoints() {
	if (!prototype) {
		buildBeam();
		prototype = make_drawelement(std::string("checkpoint_prototype"));
		auto mesh = make_mesh(std::string("checkpoint_mesh"));
		mesh->add_vertex_buffer(GL_FLOAT, 3, 64, verts.data(), GL_STATIC_DRAW);
		mesh->add_index_buffer(66, indices.data());
		mesh->set_primitive_type(GL_TRIANGLE_STRIP);
		prototype->add_mesh(mesh);

		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(0,0,0), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(-20,40,0), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(0,60,-20), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(20,80,0), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(0,100,30), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(-40,120,20), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(-20,130,-40), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(40,140,-30), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(40,160,50), INFINITY));
		
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(-362,370,49), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(-386,430,-56), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(-250,450,33), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(199,660,76), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(181,690,-95), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(293,640,-53), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(349,720,-94), INFINITY));
		checkpoints.push_back(make_shared<checkpoint>(ids++, vec3(320,800,60), INFINITY));
		closest_checkpoint_to_player = checkpoints[0];
	} else {
		cerr << "did you just call init_checkpoints() twice?!" << endl;
	}
}

// build mesh
void Checkpoints::buildBeam() {
	for (int i = 0; i < 32; i++) {
		float angle = 2 * i * M_PI / 32;
		verts.push_back(glm::vec3(cos(angle), 0, sin(angle)));
		verts.push_back(glm::vec3(cos(angle), 6, sin(angle)));

	}
	for (int i = 0; i < 64; i++) {
		indices.push_back(i);
	}
	indices.push_back(0);
	indices.push_back(1);
}

void Checkpoints::update_distances_to_player() {
	closest_checkpoint_to_player->distance_to_player = INFINITY;
	for(auto &checkp : checkpoints) {
		checkp->distance_to_player = distance(the_player3d->pos, checkp->pos + glm::vec3(0, (the_player3d->characterHeight + the_player3d->characterWidth * 2.f) * 0.5f,0));
		if(checkp->distance_to_player <= closest_checkpoint_to_player->distance_to_player) closest_checkpoint_to_player = checkp;
	}
}

bool Checkpoints::show_overlay_allowed() {
	return closest_checkpoint_to_player->distance_to_player < checkpoint_distance_threshold;
}

void Checkpoints::update() {
	update_distances_to_player();
	// ImGui::Begin("Checkpoint Debug");
	// ImGui::DragFloat("threshold", &checkpoint_distance_threshold);
	// ImGui::Text("closest_checkpoint_to_player: %d", closest_checkpoint_to_player->id);
	// ImGui::End();
	if (debug_gui) {
		// static int checkpoint_id = 0;
		// static float x = 0, y = 0, z = 0;
		// ImGui::Begin("Checkpoint-Setter");
		// ImGui::SliderInt("checkpoint_id", &checkpoint_id, 0, 9);
		// ImGui::SliderFloat("x", &x, -100, 100);
		// ImGui::SliderFloat("y", &y, -100, 100);
		// ImGui::SliderFloat("z", &z, -100, 100);
		// ImGui::End();
		// checkpoints[checkpoint_id]->pos = vec3(x, y, z);
	}
}

bool compare_distances(shared_ptr<Checkpoints::checkpoint> c1, shared_ptr<Checkpoints::checkpoint> c2) {
	return distance(Camera::current()->pos, c1->pos) > distance(Camera::current()->pos, c2->pos);
}

void Checkpoints::draw(const std::shared_ptr<Shader> &shader) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	prototype->use_shader(shader);
	prototype->bind();
	setup_light(prototype->shader);
//	enable the below line when checkpoints are placed correctly to enable correct blending
	auto tmp = checkpoints;
	sort(tmp.begin(), tmp.end(), compare_distances);
	for (auto &c : tmp) {
		prototype->shader->uniform("cp_pos", c->pos);
		prototype->draw(glm::translate(c->pos));
	}
	prototype->unbind();
	glDisable(GL_BLEND);
}
