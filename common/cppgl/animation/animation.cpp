#include "animation.h"
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext.hpp>

using namespace std;
using namespace glm;

Animation::Animation(int id, string name, float length, vector<std::shared_ptr<KeyFrame>> frames) : id(id), name(name), length(length), frames(frames) {

}

Animation::Animation(int id, std::string name, float length) : id(id), name(name), length(length) {

}

float Animation::get_length() {
	return length;
}

unordered_map<int, mat4> Animation::calculate_current_animation_pose(float curr_animation_time) {
	animation_time = curr_animation_time;
	unordered_map<int, mat4> poses;
	for (auto &ba : bone_animations) {
		pair<std::shared_ptr<KeyFrame>, std::shared_ptr<KeyFrame>> kframes = ba.second->get_previous_and_next_keyframes(animation_time);
		float alpha = calculate_alpha(kframes);
		poses[ba.second->bone_id] = interpolate_poses(kframes, alpha);
	}
	return poses;
}

float Animation::calculate_alpha(std::pair<std::shared_ptr<KeyFrame>, std::shared_ptr<KeyFrame>> kframes) {
	float total_time = kframes.second->get_time_stamp() - kframes.first->get_time_stamp();
	float current_time = animation_time - kframes.first->get_time_stamp();
	return current_time / total_time;
}

mat4 Animation::interpolate_poses(std::pair<std::shared_ptr<KeyFrame>, std::shared_ptr<KeyFrame>> frames, float alpha) {
	unordered_map<int, mat4> currentPose;
	BoneTransform previous_trafo = frames.first->trafo;
	BoneTransform next_trafo = frames.second->trafo;
	shared_ptr<BoneTransform> currentTransform = BoneTransform::interpolate(previous_trafo, next_trafo, alpha);
	return currentTransform->get_local_transform();
}