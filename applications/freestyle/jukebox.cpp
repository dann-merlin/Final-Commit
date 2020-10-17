#include "jukebox.h"
#include <cppgl/camera.h>
#include <ik_ISoundStopEventReceiver.h>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <chrono>
#include <thread>

using namespace std;


mutex mtx;


namespace Jukebox {

	uint sound_ids;
	irrklang::ISoundEngine *engine;

	struct static_sound {
		irrklang::ISound *isound;
		irrklang::ISoundEffectControl *effect_control;
	};

	struct ts_st {
		mutex mtx;
		unordered_map<irrklang::ISound *, bool> st;
		irrklang::ISound *lockingISound;

		void insert(irrklang::ISound *isound) {
			mtx.lock();
			st[isound] = true;
			mtx.unlock();
		}

		void free(irrklang::ISound *isound) {
			while (true) {
				mtx.lock();
				if (st.erase(isound) != 0) {
					isound->drop();
					st[isound] = false;
					break;
				}
				mtx.unlock();
				/* this_thread::sleep_for(chrono::seconds(1)); */
			}
			mtx.unlock();
		}

		bool lock(irrklang::ISound *isound) {
			mtx.lock();
			if (st[isound] == false) {
				mtx.unlock();
				return true;
			}
			lockingISound = isound;
			if (isound == nullptr) mtx.unlock();
			return isound == nullptr;
		}

		void unlock(irrklang::ISound *isound) {
			if (isound == lockingISound) mtx.unlock();
		}
	};

	ts_st sounds_playing;
	unordered_map<sound, irrklang::ISoundSource *> sound_sources;
	unordered_map<string, sound> sound_map;
	vector<sound> steps{sound::step1, sound::step2, sound::step3, sound::step4};
	vector<sound> static_sounds_list{jetpack_thrust, world_music, world_music_damped, battle_music, wind};
	unordered_map<sound, static_sound> static_sounds;
	float master_volume = 1.f;
	bool mute = false;

	// Event Listener
	void free_helper(ISound *isound) {
		sounds_playing.free(isound);
	}

	class mySoundStopEventReceiver : public irrklang::ISoundStopEventReceiver {
	public:
		virtual void OnSoundStopped(irrklang::ISound *sound, irrklang::E_STOP_EVENT_CAUSE reason, void *userData) {
			thread t(&free_helper, sound);
			t.detach();
		}
	};

	mySoundStopEventReceiver *msser = new mySoundStopEventReceiver;


	// init
	void init_jukebox() {
		// init engine
		engine = irrklang::createIrrKlangDevice();

		// load all files
		sound_sources[sound::jetpack_thrust] = engine->addSoundSourceFromFile("render-data/sounds/jetpack_louder.wav");
		sound_sources[sound::rope_pull] = engine->addSoundSourceFromFile("render-data/sounds/rope_pull.wav");
		sound_sources[sound::wind] = engine->addSoundSourceFromFile("render-data/sounds/seamless_wind.wav");
		sound_sources[sound::ground_hit] = engine->addSoundSourceFromFile("render-data/sounds/landing.wav");
		sound_sources[sound::step1] = engine->addSoundSourceFromFile("render-data/sounds/step1.wav");
		sound_sources[sound::step2] = engine->addSoundSourceFromFile("render-data/sounds/step2.wav");
		sound_sources[sound::step3] = engine->addSoundSourceFromFile("render-data/sounds/step3.wav");
		sound_sources[sound::step4] = engine->addSoundSourceFromFile("render-data/sounds/step4.wav");
		sound_sources[beep] = engine->addSoundSourceFromFile("render-data/sounds/wet_click.wav");
		sound_sources[grappling] = engine->addSoundSourceFromFile("render-data/sounds/grappling.wav");
		sound_sources[pickup] = engine->addSoundSourceFromFile("render-data/sounds/pickup.wav");
		sound_sources[world_music] = engine->addSoundSourceFromFile("render-data/sounds/songs/exitthepremises.wav");
		sound_sources[battle_music] = engine->addSoundSourceFromFile("render-data/sounds/songs/Race_Car.wav");
		sound_sources[world_music_damped] = engine->addSoundSourceFromFile("render-data/sounds/songs/exitthepremisesdamped.wav");

		sound_sources[player_damage] = engine->addSoundSourceFromFile("render-data/sounds/player_damage.wav");
		sound_sources[player_default] = engine->addSoundSourceFromFile("render-data/sounds/player_default.wav");
		sound_sources[player_go] = engine->addSoundSourceFromFile("render-data/sounds/player_go.wav");
		sound_sources[sl_damage] = engine->addSoundSourceFromFile("render-data/sounds/sl_damage.wav");
		sound_sources[sl_blow] = engine->addSoundSourceFromFile("render-data/sounds/sl_terminal_velocity.wav");
		sound_sources[padding_damage] = engine->addSoundSourceFromFile("render-data/sounds/padding_damage.wav");
		sound_sources[zbf_damage] = engine->addSoundSourceFromFile("render-data/sounds/zbf_damage.wav");
//		sound_sources[zbf_extra_damage] = engine->addSoundSourceFromFile("render-data/sounds/zbf_extra_damage.wav");

		sound_map["jetpack"] = jetpack_thrust;
		sound_map["rope"] = rope_pull;
		sound_map["wind"] = wind;
		sound_map["ground_hit"] = ground_hit;
		sound_map["step1"] = step1;
		sound_map["step2"] = step2;
		sound_map["step3"] = step3;
		sound_map["step4"] = step4;
		sound_map["beep"] = beep;
		sound_map["grappling"] = grappling;
		sound_map["pickup"] = pickup;
		sound_map["world_music"] = world_music;

		sound_map["Player_damage"] = player_damage;
		sound_map["Player_default"] = player_default;
		sound_map["Player_go"] = player_go;
		sound_map["sl_damage"] = sl_damage;
		sound_map["sl_Terminal Velocity"] = sl_blow;
		sound_map["Padding_damage"] = padding_damage;
		sound_map["Z-Buffer-Fighting_damage"] = zbf_damage;

		// start static sounds paused
		for (auto &s : static_sounds_list) {
			static_sounds[s].isound = engine->play3D(sound_sources[s], irrklang::vec3df(0, 0, 0), true, true, true, true);
			static_sounds[s].effect_control = static_sounds[s].isound->getSoundEffectControl();
		}

		sound_ids = 0;
	}

	// free
	void destroy_jukebox() {
		engine->drop();
	}

	void play_static_sound(sound s) {
		auto isound = static_sounds[s].isound;
		isound->setIsPaused(false);
	}

	void pause_static_sound(sound s) {
		auto isound = static_sounds[s].isound;
		isound->setIsPaused(true);
	}

	void set_volume_static_sound(sound s, float volume) {
		auto isound = static_sounds[s].isound;
		isound->setVolume(volume);
	}

	void damp_music(bool enable) {
		if(enable) {
			auto isound_normal = static_sounds[world_music].isound;
			isound_normal->setIsPaused(true);
			auto isound_damp = static_sounds[world_music_damped].isound;
			float time_p = isound_normal->getPlayPosition();
			isound_damp->setPlayPosition(time_p);
			isound_damp->setIsPaused(false);
		} else {
			auto isound_damp = static_sounds[world_music_damped].isound;
			isound_damp->setIsPaused(true);
			auto isound_normal = static_sounds[world_music].isound;
			float time_p = isound_damp->getPlayPosition();
			isound_normal->setPlayPosition(time_p);
			isound_normal->setIsPaused(false);

		}
	}

	void restart_static_sound(sound s) {
		auto isound = static_sounds[s].isound;
		isound->setPlayPosition(0);
		isound->setIsPaused(false);
		isound->setPlayPosition(0);
	}

	void play_jetpack() {
		play_static_sound(sound::jetpack_thrust);
	}

	void pause_jetpack() {
		pause_static_sound(sound::jetpack_thrust);
	}

	void pause_sound(ISound *sound_id) {
		if (sounds_playing.lock(sound_id)) return;
		sound_id->setIsPaused(true);
		sounds_playing.unlock(sound_id);
	}

	void stop_sound(ISound *sound_id) {
		if (sound_id == nullptr) return;
		if (sounds_playing.lock(sound_id)) return;
		sound_id->stop();
		sounds_playing.unlock(sound_id);
	}

	ISound *play_sound(sound s, bool loop, bool start_paused) {
		return play_sound_effects(s, loop, start_paused, false);
	}

	ISound *play_sound_string(string s) {
//		return nullptr;
		cout << "Jukebox::play_sound_string( " << s << " )" << endl;
		if (sound_map.count(s) > 0) {
			return play_sound(sound_map[s]);
		} else if (s.find("Player") != string::npos) {
			if (s.find("cout") != string::npos || s.find("Calculate Normals") != string::npos) {
				return play_sound(player_go);
			} else {
				return play_sound(player_default);
			}
		} else if (s.find("Z-Buffer-Fighting") != string::npos && s.find("damage") != string::npos) {
			return play_sound(zbf_damage);
		} else {
			cerr << "sound '" << s << "' was not found" << endl;
			return nullptr;
		}
	}

	ISound *play_sound_effects(sound s, bool loop, bool start_paused, bool enable_sound_effects) {
		irrklang::ISound *isound = engine->play2D(sound_sources[s], loop, true, true, enable_sound_effects);
		sounds_playing.insert(isound);
		isound->setSoundStopEventReceiver(msser);
		isound->setIsPaused(start_paused);

		return isound;
	}

	ISound *play_sound_3D(sound s, glm::vec3 location, bool loop, bool start_paused) {
		return play_sound_3D_effects(s, location, loop, start_paused, false);
	}

	ISound *play_sound_3D_effects(sound s, glm::vec3 location, bool loop, bool start_paused, bool enable_sound_effects) {
		irrklang::ISound *isound = engine->play3D(sound_sources[s], irrklang::vec3df(location.x, location.y, location.z), loop, true, true, enable_sound_effects);
		sounds_playing.insert(isound);
		isound->setSoundStopEventReceiver(msser);
		isound->setIsPaused(start_paused);
		return isound;
	}

	void toggle_mute() {
		if (mute) {
			engine->setSoundVolume(master_volume);
			mute = false;
		} else {
			master_volume = engine->getSoundVolume();
			engine->setSoundVolume(0);
			mute = true;
		}
	}

	void update_cam_pos_for_jukebox(glm::vec3 pos, glm::vec3 dir, glm::vec3 vel, glm::vec3 up) {
		engine->setListenerPosition(irrklang::vec3df(pos.x, pos.y, pos.z), irrklang::vec3df(dir.x, dir.y, dir.z), irrklang::vec3df(vel.x, vel.y, vel.z), irrklang::vec3df(up.x, up.y, up.z));
	}

	void play_beep() {
		play_sound(beep);
	}
}
