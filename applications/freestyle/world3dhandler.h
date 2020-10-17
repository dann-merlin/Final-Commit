#pragma once
#include "player3d.h"
#include "rope.h"
#include "player.h"
#include "world.h"
#include "save.h"
#include "files.h"
#include "soup.h"
#include <memory>

class World3DHandler {
	public:
		static std::shared_ptr<Physic> physics;
		static std::shared_ptr<World> world;
		static std::shared_ptr<Files> the_files;
		static std::shared_ptr<Texture2D> the_crosshair;

		World3DHandler();
		void start_3D(std::shared_ptr<save_t> save);

		void update();

		static bool get_interactive_mode();
		static void reboot();
		static void shutdown();
		static void try_to_activate_checkpoint_overlay();
		static void deactivate_checkpoint_overlay(bool activate_music = true);
		void load_save(std::shared_ptr<save_t> save);
		void generate_rope();

		void draw();
		void gbuf_draw();
};
