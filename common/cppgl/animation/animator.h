#ifndef CPPGL_ANIMATOR_H
#define CPPGL_ANIMATOR_H

#include "animation.h"
#include "bone.h"

class Animator {
public:
	std::unordered_map<std::string, std::shared_ptr<Animation>> animations;
	float animation_time = 0;
	std::string current_animation = "no_anim";

	Animator();

	void do_animation(std::string anim);

	void update(std::shared_ptr<Bone> &root_bone);

	void increase_animation_time();

	std::unordered_map<int, glm::mat4> calculate_current_animation_pose();

	static void apply_pose_to_bone(std::unordered_map<int, glm::mat4> &current_pose, std::shared_ptr<Bone> &bone, glm::mat4 parent_transform);
};


#endif //CPPGL_ANIMATOR_H
