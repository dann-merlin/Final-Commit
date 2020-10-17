#ifndef CPPGL_KEYFRAME_H
#define CPPGL_KEYFRAME_H

#include "bonetransform.h"
#include <unordered_map>

class KeyFrame {
public:
	float time_stamp;
	std::unordered_map<int, BoneTransform> transform;
	BoneTransform trafo;

	KeyFrame(float time_stamp, std::unordered_map<int, BoneTransform> transform);
	KeyFrame(float time_stamp, BoneTransform trafo);

	float get_time_stamp();

};


#endif //CPPGL_KEYFRAME_H
