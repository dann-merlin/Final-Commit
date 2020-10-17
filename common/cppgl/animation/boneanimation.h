#ifndef CPPGL_BONEANIMATION_H
#define CPPGL_BONEANIMATION_H

#include <vector>
#include <memory>
#include "keyframe.h"

class BoneAnimation {
public:
	const int bone_id;
	const float length;
	const std::vector<std::shared_ptr<KeyFrame>> frames;

	BoneAnimation(int bone_id, float length, std::vector<std::shared_ptr<KeyFrame>> &frames);

	float get_length();

	std::pair<std::shared_ptr<KeyFrame>, std::shared_ptr<KeyFrame>> get_previous_and_next_keyframes(float animation_time);
};


#endif //CPPGL_BONEANIMATION_H
