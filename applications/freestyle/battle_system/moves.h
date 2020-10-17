#ifndef CPPGL_MOVES_H
#define CPPGL_MOVES_H

#include <string>
#include <unordered_map>
#include <memory>
#include "fighter.h"

class Moves {
public:
	typedef std::pair<Fighter::stats_t, Fighter::stats_t> stat_diff_t;
	struct single_attack_t {
		std::string attack_name; // attack_names von p1 und p2
		stat_diff_t stat_change;
		int effective;
	};

	typedef single_attack_t (*move_function)(std::shared_ptr<Fighter>, std::shared_ptr<Fighter>);

	enum move_type {
		CPU_t, GPU_t, dual_t
	};
	struct move {
		std::string name = "no_move_with_that_name";
		move_type t = dual_t;
		int power = 0;
		int accuracy = 100;
		move_function func = move_default;
	};

	static std::unordered_map<std::string, move> all_moves;

	static void init_moves();

	static int calc_damage(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target, move m);

	// Player moves:

	static single_attack_t move_glPolygonOffset(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	static single_attack_t move_calculate_normals(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	static single_attack_t move_hot_fix(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	static single_attack_t move_cout_debugging(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	static single_attack_t move_hppp(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	static single_attack_t move_off_on(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	// Enemy moves:

	//padding
	static single_attack_t move_arm_smash(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	static single_attack_t move_defensive_stance(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	static single_attack_t move_body_slam(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	//sl
	static single_attack_t move_plow(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	static single_attack_t move_terminal_velocity(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	static single_attack_t move_smoke_bomb(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	// zbf
	static single_attack_t move_fight_me(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	static single_attack_t move_z_buffs(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	static single_attack_t move_wall_of_ugly(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	static single_attack_t move_default(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target);

	static single_attack_t default_attack(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target, std::string move_name);
};


#endif //CPPGL_MOVES_H
