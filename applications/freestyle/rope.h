#ifndef CPPGL_ROPE_H
#define CPPGL_ROPE_H

#include <cppgl/drawelement.h>
#include "physic.h"
#include "curves/CatmullRom.h"

class Rope {
public:
	// data
	std::vector<glm::vec3> rope_verts;
	std::vector<glm::vec3> prev_ends;
	uint rope_buffer_id;
	btSoftBody *psb0;
	std::shared_ptr<Drawelement> prototype;
	std::shared_ptr<CatmullRom> bezier;

	Rope();

	void add_rope(btRigidBody* player, glm::vec3 anchor);
	void remove_rope();
	void draw(const std::shared_ptr<Shader> &shader);
};


#endif //CPPGL_ROPE_H
