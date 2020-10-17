#include "bonetransform.h"
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;


BoneTransform::BoneTransform() : pos(0), rot(1, 0, 0, 0), scale(1) {
	cerr << "called BoneTransform() without parameters" << endl;
}

BoneTransform::BoneTransform(vec3 pos, quat rot, vec3 scale) : pos(pos), rot(rot), scale(scale) {
}

mat4 BoneTransform::get_local_transform() {
	mat4 trafo_trans = glm::translate(mat4(1), pos);
	mat4 trafo_scale = glm::scale(mat4(1), scale);
	mat4 trafo_rot = mat4_cast(rot);
	return trafo_trans * trafo_rot * trafo_scale;
}

shared_ptr<BoneTransform> BoneTransform::interpolate(BoneTransform frameA, BoneTransform frameB, float a) {
	vec3 inter_pos = (1 - a) * frameA.pos + a * frameB.pos;
	quat inter_rot = slerp(frameA.rot, frameB.rot, a);
	vec3 inter_scale = (1 - a) * frameA.scale + a * frameB.scale;
	auto inter = make_shared<BoneTransform>(inter_pos, inter_rot, inter_scale);
	return inter;
}