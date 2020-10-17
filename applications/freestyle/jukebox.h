#pragma once
#ifndef CPPGL_JUKEBOX_H
#define CPPGL_JUKEBOX_H

#include <SDL2/SDL.h>
#include <iostream>
#include <unordered_map>
#include <unistd.h>
#include <sys/types.h>
#include <atomic>
#include <irrKlang.h>
#include <memory>
#include <glm/glm.hpp>

using namespace std;

namespace Jukebox {
	// data
	typedef irrklang::ISound ISound;
	enum sound {
		jetpack_thrust,
		rope_pull,
		wind,
		ground_hit,
		grappling,
		pickup,
		world_music,
		world_music_damped,
		step1,
		step2,
		step3,
		step4,
		beep,
		battle_music,
		player_damage,
		padding_damage,
		sl_damage,
		zbf_damage,
		zbf_extra_damage,
		player_default,
		player_go,
		sl_blow
	};

	//functions
	void init_jukebox();
	void destroy_jukebox();
	irrklang::ISound *play_sound(sound s, bool loop = false, bool start_paused = false);
	irrklang::ISound *play_sound_string(string s);
	irrklang::ISound *play_sound_effects(sound s, bool loop, bool start_paused, bool enable_sound_effects);
	irrklang::ISound *play_sound_3D(sound s, glm::vec3 location, bool loop, bool start_paused);
	irrklang::ISound *play_sound_3D_effects(sound s, glm::vec3 location, bool loop, bool start_paused, bool enable_sound_effects);
	void update_cam_pos_for_jukebox(glm::vec3 pos, glm::vec3 dir, glm::vec3 vel, glm::vec3 up);
	void toggle_mute();
	void damp_music(bool enable);
	void stop_sound(ISound * sound_id);
	void play_static_sound(sound s);
	void pause_static_sound(sound s);
	void set_volume_static_sound(sound s, float volume);
	void restart_static_sound(sound s);
	void play_jetpack();
	void pause_jetpack();



	// specific sounds
	void play_beep();

}
#endif //CPPGL_JUKEBOX_H
