#include "bone.h"

using namespace glm;

mat4 Bone::global_inverse_trafo = mat4(1);

Bone::Bone(int id, std::string name, mat4 local_def_transform) : id(id), name(name) {
	local_default_trafo = local_def_transform;
}

void Bone::add_child(std::shared_ptr<Bone> child) {
	if (id != child->id)
		children[child->id] = child;
}

void Bone::set_animated_trafo(mat4 anim_trafo) {
	animated_trafo = anim_trafo;
}

mat4 Bone::get_animated_trafo() {
	return animated_trafo;
}

