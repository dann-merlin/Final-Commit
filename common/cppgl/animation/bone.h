#ifndef CPPGL_BONE_H
#define CPPGL_BONE_H
#pragma once

#include <unordered_map>
#include <string>
#include <cppgl/cppgl.h>


class Bone {
public:
	int id;
	std::string name;
	glm::mat4 local_default_trafo, animated_trafo, inverse_default_trafo;
	std::unordered_map<int, std::shared_ptr<Bone>> children;
	static glm::mat4 global_inverse_trafo;

	Bone(int id, std::string name, glm::mat4 local_trafo);

	void add_child(std::shared_ptr<Bone> child);

	void set_animated_trafo(glm::mat4 anim_trafo);

	glm::mat4 get_animated_trafo();


};


#endif //CPPGL_BONE_H
