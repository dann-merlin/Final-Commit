#include "files.h"
#include "world3dhandler.h"
#include "battle_system/battlehandler.h"
#include "rendering.h"
#include "jukebox.h"
#include <cppgl/meshloader.h>
#include <glm/ext/matrix_transform.hpp>
#include "save.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

uint Files::ids = 0;
list<shared_ptr<Files::file>> Files::files, Files::collected, Files::commited;
vector<shared_ptr<Files::file>> Files::all_files;
std::vector<std::shared_ptr<Drawelement>> Files::prototype;
extern std::shared_ptr<Player3D> the_player3d;
extern shared_ptr<save_t> current_save;
extern bool super_easy;

Files::Files(const std::vector<std::shared_ptr<Drawelement>> &proto) {
	prototype = MeshLoader::load("render-data/files/file.obj", false);
	for (const auto &elem : prototype) {
		elem->material->add_texture("specular", make_texture("file_spec", "render-data/files/file_spec.png"));
	}
	if (prototype.empty()) prototype = proto;
	/*
	files.push_back(make_shared<file>(ids++, 1, vec3(0, 2, 0), 0));
	files.push_back(make_shared<file>(ids++, 7, vec3(0, 14, 0), 0));
	files.push_back(make_shared<file>(ids++, 15, vec3(0, 30, 0), 0));
	files.push_back(make_shared<file>(ids++, 29, vec3(0, 58, 0), 0));
	files.push_back(make_shared<file>(ids++, 42, vec3(0, 84, 0), 0));
	files.push_back(make_shared<file>(ids++, 57, vec3(0, 114, 0), 0));
	files.push_back(make_shared<file>(ids++, 61, vec3(0, 122, 0), 0));
	files.push_back(make_shared<file>(ids++, 72, vec3(0, 144, 0), 0));
	files.push_back(make_shared<file>(ids++, 86, vec3(0, 172, 0), 0));
	files.push_back(make_shared<file>(ids++, 100, vec3(0, 200, 0), 0));
	*/
	files.push_back(make_shared<file>(ids++, 1, vec3(-37, 2, 3), 0, "world3dhandler.cpp"));
	files.push_back(make_shared<file>(ids++, 6, vec3(-11, 57, 45), 1, "save.cpp"));
	files.push_back(make_shared<file>(ids++, 12, vec3(0, 71, 0), 2, "rope.cpp"));
	files.push_back(make_shared<file>(ids++, 19, vec3(81, 107, -37), 0, "jukebox.cpp"));
	files.push_back(make_shared<file>(ids++, 25, vec3(324, 109, 114), 1, "files.cpp"));
	files.push_back(make_shared<file>(ids++, 31, vec3(161, 176, 104), 2, "soup.fs"));
	files.push_back(make_shared<file>(ids++, 37, vec3(-185, 234, 0), 1, "skybox.cpp"));
	files.push_back(make_shared<file>(ids++, 43, vec3(-357, 413, 1), 1, "player3d.cpp"));
	files.push_back(make_shared<file>(ids++, 50, vec3(-211, 431, -118), 0, "checkpoints.cpp"));
	files.push_back(make_shared<file>(ids++, 56, vec3(6, 505, 365), 0, "main.cpp"));
	files.push_back(make_shared<file>(ids++, 63, vec3(15, 522, 0), 2, "animatedmesh.cpp"));
	files.push_back(make_shared<file>(ids++, 69, vec3(-73, 543, -393), 0, "moves.cpp"));
	files.push_back(make_shared<file>(ids++, 75, vec3(289, 607, 86), 2, "matrixsky.fs"));
	files.push_back(make_shared<file>(ids++, 86, vec3(15, 650, -210), 1, "battlehandler.cpp"));
	files.push_back(make_shared<file>(ids++, 91, vec3(222, 706, -85), 2, "rendering.cpp"));
	files.push_back(make_shared<file>(ids++, 100, vec3(330, 816, -3), 1, "gui.cpp"));
	all_files.insert(all_files.begin(), files.begin(), files.end());
}

shared_ptr<map<int64_t, shared_ptr<Files::file>>> Files::make_collected_files_map() {
	auto m = make_shared<map<int64_t, shared_ptr<Files::file>>>();
	int64_t j = 0;
	for(auto &entry : collected) {
		(*m)[j++] = entry;
	}
	return m;
}

void Files::update() {
	// for (auto it = files.begin(); it != files.end(); it++) {
	// 	auto f = *it;
	// 	if (distance(f->pos, World3DHandler::the_player3d->pos) < collect_dist) {
	// 		collected.push_back(f);
	// 		files.erase(it);
	// 	}
	// }
	static float time = 0;
	time += Context::frame_time() * 0.001;
	for (uint i = 0; i < files.size(); i++) {

		auto t = files.begin();
		advance(t, i);
		// float angle = sin(Context::frame_time() + i) * 0.01;
		auto trans_trafo = translate((*t)->pos);
		auto rotate_trafo = rotate((time + i * 0.7845f) * ((float) fmod(i+1,0.397f) + 0.75f ) * (( i % 2 - 0.5f) * 2), vec3(0,1,0));
		// (*t)->trafo = glm::rotate((*t)->trafo, angle, vec3(0,1,0));
		(*t)->trafo = trans_trafo * rotate_trafo * mat4(1);
		if (distance((*t)->pos, the_player3d->pos) < collect_dist || super_easy) {
			collected.push_back((*t));
			files.erase(t);
			Jukebox::play_sound(Jukebox::sound::pickup);
			current_save->apply_current_files();
		}// else { i++; };
	}
}

void Files::draw(const std::shared_ptr<Shader> &shader) {
	for (auto &elem : prototype) {
		elem->use_shader(shader);
		elem->bind();
		setup_light(elem->shader);
		for (auto &f :files) {
			// elem->draw(glm::translate(f->pos));
			elem->draw(f->trafo);
		}
		elem->unbind();
	}
}
