#ifndef CPPGL_CHECKPOINTS_H
#define CPPGL_CHECKPOINTS_H

#include <cppgl/drawelement.h>

class Checkpoints {
public:
	struct checkpoint {
		int id;
		glm::vec3 pos;
		float distance_to_player;
		checkpoint(uint id_p, glm::vec3 pos_p, float dist) : id(id_p), pos(pos_p), distance_to_player(dist) {}
	};

	static std::vector<std::shared_ptr<checkpoint>> checkpoints;
	static std::shared_ptr<Drawelement> prototype;
	static std::vector<glm::vec3> verts;
	static std::vector<unsigned int> indices;
	static uint ids;
	static const float collect_dist;
	static std::shared_ptr<checkpoint> closest_checkpoint_to_player;

	Checkpoints();

	static void update();

	static void draw(const std::shared_ptr<Shader> &shader);

	static void buildBeam();

	static void init_checkpoints();

	static void update_distances_to_player();
	static bool show_overlay_allowed();
};


#endif //CPPGL_CHECKPOINTS_H
