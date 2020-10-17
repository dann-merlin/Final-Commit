#ifndef CPPGL_ANIMATION_H
#define CPPGL_ANIMATION_H

#include <vector>
#include <unordered_map>
#include <stdexcept>
#include "boneanimation.h"

class Animation {
public:
	int id;
	std::string name;
	const float length;
	const std::vector<std::shared_ptr<KeyFrame>> frames;
	std::unordered_map<int, std::shared_ptr<BoneAnimation>> bone_animations;
	float animation_time;

	Animation(int id, std::string name, float length, std::vector<std::shared_ptr<KeyFrame>> frames);

	Animation(int id, std::string name, float length);

	float get_length();

	std::unordered_map<int, glm::mat4> calculate_current_animation_pose(float animation_time);

	float calculate_alpha(std::pair<std::shared_ptr<KeyFrame>, std::shared_ptr<KeyFrame>> frames);

	static glm::mat4 interpolate_poses(std::pair<std::shared_ptr<KeyFrame>, std::shared_ptr<KeyFrame>> frames, float alpha);
};


#endif //CPPGL_ANIMATION_H
