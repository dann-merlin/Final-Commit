#include "animator.h"
#include "../../../applications/freestyle/battle_system/battlehandler.h"
#include <cppgl/context.h>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>


using namespace glm;
using namespace std;

extern bool debug_gui;

Animator::Animator() {

}

void Animator::do_animation(string anim) {
	if (anim != "idle" || current_animation != "idle") {
		animation_time = 0;
	}
	if (animations.count(anim) > 0) {
		current_animation = anim;
	} else if (anim.find("damage") != string::npos && animations.count("damage") > 0) {
		current_animation = "damage";
	} else {
		cerr << "Animator: animation '" << anim << "' was not found" << endl;
		current_animation = "default_attack";
	}
}

void Animator::update(shared_ptr<Bone> &root_bone) {
	if (current_animation == "no_anim") {
		cerr << "current_animation was no_anim" << endl;
		return;
	}
	increase_animation_time();
	//debug
	if (debug_gui) {
		static float time = 1;
		static bool use_slider = false;
		ImGui::Begin("animation_time");
		ImGui::Checkbox("Use slider", &use_slider);
		if (animations.count(current_animation) == 0) {
			cerr << "Animation " << current_animation << " not found in update()" << endl;
			return;
		}
		ImGui::SliderFloat("anim_time", &time, 0, animations[current_animation]->length - 0.05);
		ImGui::End();
		if (use_slider) {
			animation_time = time;
		}
	}
	// debug end
	unordered_map<int, mat4> currentPose = calculate_current_animation_pose();
	apply_pose_to_bone(currentPose, root_bone, mat4(1));
}

void Animator::increase_animation_time() {
	animation_time += Context::frame_time() / 1000;
	if (animations.count(current_animation) == 0) {
		cerr << "Animation " << current_animation << " not found in update()" << endl;
		return;
	}
	if (animation_time > animations[current_animation]->get_length()) {
		animation_time = fmod(animation_time, animations[current_animation]->get_length());
		if (current_animation != "idle" && current_animation.find("damage") == string::npos && current_animation.find("effect") == string::npos) {
			BattleHandler::states.pop();
			BattleHandler::new_in_state = true;
		}
	}
}

unordered_map<int, mat4> Animator::calculate_current_animation_pose() {
	if (animations.count(current_animation) == 0) { cerr << "Animation " << current_animation << " not found in update()" << endl; }
	return animations[current_animation]->calculate_current_animation_pose(animation_time);
}

void Animator::apply_pose_to_bone(unordered_map<int, mat4> &current_pose, shared_ptr<Bone> &bone, mat4 parent_transform) {

	mat4 curr_local_transform = current_pose[bone->id];
	if (curr_local_transform == mat4(0)) {
		curr_local_transform = mat4(1);
		cerr << "Animator: curr_local_transform of " << bone->id << " was mat4(0). This should not have happened" << endl;
	}
//	cout << bone->id << endl;
//	cout << glm::to_string(curr_local_transform) << endl;
	mat4 curr_transform = parent_transform * curr_local_transform;
	for (auto &child : bone->children) {
		apply_pose_to_bone(current_pose, child.second, curr_transform);
	}
	curr_transform = Bone::global_inverse_trafo * curr_transform * bone->local_default_trafo;// * glm::rotate(mat4(1), 90.0f, vec3(1,0,0));
	bone->set_animated_trafo(curr_transform);
}
