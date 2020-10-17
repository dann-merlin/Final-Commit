#ifndef CPPGL_FILES_H
#define CPPGL_FILES_H

#include "battle_system/fighter.h"
#include "gui.h"
#include <iostream>
#include <list>

class Files {
public:
	struct file {
		public:
		uint id;
		uint size;
		std::string name;
		glm::vec3 pos;
		glm::mat4 trafo;
		uint enemy_id;
		bool deleted = false; // DONT USE THIS VARIABLE. IT IS NONSENSE BUT I NEED IT FOR GUI
		file(uint id_p, uint size_p, glm::vec3 pos_p, uint enemy_id_p, std::string name_p) {
			id = id_p;
			size = size_p;
			pos = pos_p;
			enemy_id = enemy_id_p;
			name = name_p;
			trafo = glm::mat4(1);
			trafo[3] = glm::vec4(pos, 1);
		}

		bool draw() { // DONT USE THIS OUTSIDE OF GUI
			ImGui::TextWrapped("Bug: ???");
			ImGui::TextWrapped("Filesize: %d", size);
			bool b = ImGui::Button(("Commit File##" + std::to_string(id)).c_str());
			return b;
		}

		std::string get_collapse_header() {
			return std::to_string(size) + "kb: " + name;
		}
	};

	static std::list<std::shared_ptr<file>> files, collected, commited;
	static std::vector<std::shared_ptr<file>> all_files;
	static std::vector<std::shared_ptr<Drawelement>> prototype;
	static uint ids;
	const float collect_dist = 6;
	const int num_files = 10;

	Files(const std::vector<std::shared_ptr<Drawelement>> &prototype);

	void update();

	static std::shared_ptr<std::map<int64_t, std::shared_ptr<file>>> make_collected_files_map();
	void draw(const std::shared_ptr<Shader> &shader);
};


#endif //CPPGL_FILES_H
