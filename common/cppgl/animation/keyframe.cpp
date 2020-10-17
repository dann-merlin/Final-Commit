#include "keyframe.h"

using namespace glm;

KeyFrame::KeyFrame(float time_stamp, std::unordered_map<int, BoneTransform> transform) : time_stamp(time_stamp), transform(transform) {

}

KeyFrame::KeyFrame(float time_stamp, BoneTransform trafo) : time_stamp(time_stamp), trafo(trafo) {

}

float KeyFrame::get_time_stamp() {
	return time_stamp;
}