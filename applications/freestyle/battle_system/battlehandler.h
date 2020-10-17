#ifndef CPPGL_BATTLEHANDLER_H
#define CPPGL_BATTLEHANDLER_H

#include "fighter.h"
#include "moves.h"
#include <memory>
#include <queue>

class BattleHandler {
public:

	struct state_t {
		std::shared_ptr<Fighter> fighter_atk;
		std::shared_ptr<Fighter> fighter_def;
		std::string text_to_display;
		std::string animation_id;
		// void (* animation_fctn)();
	};

	struct attacks_t {
//		std::pair<std::string, std::string> attack_names; // attack_names von p1 und p2
//		std::pair<Moves::stat_diff_t, Moves::stat_diff_t> stat_change;
//		std::pair<int, int> effective;
		std::pair<Moves::single_attack_t, Moves::single_attack_t> attacks;
		bool p1_is_player; // true: vec[0] is player ; false: vec[0] is enemy
	};

	static std::shared_ptr<attacks_t> att_result;
	static bool new_in_state, i_wanna_end_me, first_call;
	static std::queue<std::shared_ptr<state_t>> states;
	static std::shared_ptr<Fighter> Player, Enemy;
	static std::vector<std::shared_ptr<Fighter>> all_enemies;
	static std::vector<std::shared_ptr<Drawelement>> prototype_arena;

	// attack_t calc_attack_results(int attack_id);
	static std::shared_ptr<attacks_t> calc_attack_results(std::string attack_id_of_player);

	static std::string generate_display_action_text(bool p1);

	static std::string generate_display_stats_change_text(bool p1);

	static int check_end();

	static void generate_px_actions(std::shared_ptr<Fighter> px, int x);

	static void generate_check_end();

	static void generate_ask_okay();

	static void attack_result_to_states();

	static void init_statics();

	static void init_battle();

	static void battle();
};


#endif //CPPGL_BATTLEHANDLER_H
