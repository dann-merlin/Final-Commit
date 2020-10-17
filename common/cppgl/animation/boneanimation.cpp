#include "boneanimation.h"

using namespace std;

BoneAnimation::BoneAnimation(int bone_id, float length, vector<shared_ptr<KeyFrame>> &frames) : bone_id(bone_id), length(length), frames(frames) {

}

float BoneAnimation::get_length() {
	return length;
}

pair<std::shared_ptr<KeyFrame>, std::shared_ptr<KeyFrame>> BoneAnimation::get_previous_and_next_keyframes(float animation_time) {
	std::shared_ptr<KeyFrame> prev = frames[0];
	std::shared_ptr<KeyFrame> next = frames[0];
	for (uint i = 1; i < frames.size(); i++) {
		next = frames[i];
		if (next->get_time_stamp() > animation_time) {
			break;
		}
		prev = frames[i];
	}
	return pair<std::shared_ptr<KeyFrame>, std::shared_ptr<KeyFrame>>(prev, next);
}