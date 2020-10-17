#include "moves.h"

#include <iostream>
#include "../rendering.h"

using namespace std;

unordered_map<string, Moves::move> Moves::all_moves;
int effective = 0;

void Moves::init_moves() {
	move m;
	m.name = "glPolygonOffset";
	m.t = GPU_t;
	m.power = 10;
	m.accuracy = 100;
	m.func = move_glPolygonOffset;
	all_moves[m.name] = m;

	m.name = "Off and On again";
	m.t = dual_t;
	m.power = 20;
	m.accuracy = 100;
	m.func = move_off_on;
	all_moves[m.name] = m;

	m.name = "Calculate Normals";
	m.t = GPU_t;
	m.power = 40;
	m.accuracy = 100;
	m.func = move_calculate_normals;
	all_moves[m.name] = m;

	m.name = "HotFix";
	m.t = dual_t;
	m.power = 30;
	m.accuracy = 100;
	m.func = move_hot_fix;
	all_moves[m.name] = m;

	m.name = "cout-Debugging";
	m.t = CPU_t;
	m.power = 40;
	m.accuracy = 100;
	m.func = move_cout_debugging;
	all_moves[m.name] = m;

	m.name = "HP++";
	m.t = dual_t;
	m.power = 0;
	m.accuracy = 100;
	m.func = move_hppp;
	all_moves[m.name] = m;

	m.name = "Arm Smash";
	m.t = dual_t;
	m.power = 40;
	m.accuracy = 100;
	m.func = move_arm_smash;
	all_moves[m.name] = m;

	m.name = "Defensive Stance";
	m.t = dual_t;
	m.power = 0;
	m.accuracy = 100;
	m.func = move_defensive_stance;
	all_moves[m.name] = m;

	m.name = "Body Slam";
	m.t = dual_t;
	m.power = 60;
	m.accuracy = 100;
	m.func = move_body_slam;
	all_moves[m.name] = m;

	m.name = "Plow";
	m.t = dual_t;
	m.power = 50;
	m.accuracy = 100;
	m.func = move_plow;
	all_moves[m.name] = m;

	m.name = "Terminal Velocity";
	m.t = dual_t;
	m.power = 70;
	m.accuracy = 100;
	m.func = move_terminal_velocity;
	all_moves[m.name] = m;

	m.name = "Smoke Bomb";
	m.t = dual_t;
	m.power = 0;
	m.accuracy = 100;
	m.func = move_smoke_bomb;
	all_moves[m.name] = m;

	m.name = "Fight Me";
	m.t = dual_t;
	m.power = 30;
	m.accuracy = 100;
	m.func = move_fight_me;
	all_moves[m.name] = m;

	m.name = "Z-Buffs";
	m.t = dual_t;
	m.power = 0;
	m.accuracy = 100;
	m.func = move_z_buffs;
	all_moves[m.name] = m;

	m.name = "Wall of Ugly";
	m.t = dual_t;
	m.power = 70;
	m.accuracy = 100;
	m.func = move_wall_of_ugly;
	all_moves[m.name] = m;
}

Moves::single_attack_t Moves::move_glPolygonOffset(shared_ptr<Fighter> caster, shared_ptr<Fighter> target) {
	int damage = calc_damage(caster, target, all_moves["glPolygonOffset"]);
	stat_diff_t diff;
	Fighter::stats_t s;
	s.atk = 0;
	s.def = 0;
	s.speed = 0;
	s.hp = 0;

	diff.first = s;
	s.hp = -glm::clamp(damage, 1, 512);
	diff.second = s;
	single_attack_t attack{"glPolygonOffset", diff, effective};
	return attack;
}

Moves::single_attack_t Moves::move_calculate_normals(shared_ptr<Fighter> caster, shared_ptr<Fighter> target) {
	int damage = calc_damage(caster, target, all_moves["Calculate Normals"]);
	stat_diff_t diff;
	Fighter::stats_t s;
	s.atk = 0;
	s.def = 0;
	s.speed = 0;
	s.hp = 0;

	diff.first = s;
	s.hp = -glm::clamp(damage, 1, 512);
	diff.second = s;
	single_attack_t attack{"Calculate Normals", diff, effective};
	return attack;
}

Moves::single_attack_t Moves::move_hot_fix(shared_ptr<Fighter> caster, shared_ptr<Fighter> target) {
	int damage = calc_damage(caster, target, all_moves["HotFix"]);
	stat_diff_t diff;
	Fighter::stats_t s;
	s.atk = 0;
	s.def = 0;
	s.speed = 0;
	s.hp = 0;

	diff.first = s;
	s.hp = -glm::clamp(damage, 1, 512);
	diff.second = s;
	single_attack_t attack{"HotFix", diff, effective};
	return attack;
}

Moves::single_attack_t Moves::move_cout_debugging(shared_ptr<Fighter> caster, shared_ptr<Fighter> target) {
	float damage = calc_damage(caster, target, all_moves["cout-Debugging"]);
	stat_diff_t diff;
	Fighter::stats_t s;
	s.atk = 0;
	s.def = 0;
	s.speed = 0;
	s.hp = 0;

	diff.first = s;
	s.hp = -glm::clamp(damage, 1.0f, 512.0f);
	diff.second = s;
	single_attack_t attack{"cout-Debugging", diff, effective};
	return attack;
}

Moves::single_attack_t Moves::move_hppp(shared_ptr<Fighter> caster, shared_ptr<Fighter> target) {
	stat_diff_t diff;
	Fighter::stats_t s;
	s.atk = 0;
	s.def = 0;
	s.speed = 0;
	s.hp = 0;

	diff.second = s;
	float at_least = 0.1 * caster->max_hp + 1;
	float at_most = 0.3 * caster->max_hp + 1;
	float get = map_val(random_float(), -1, 1, at_least, at_most);
	s.hp = get;
	diff.first = s;
	single_attack_t attack{"HP++", diff, 0};
	return attack;
}

int Moves::calc_damage(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target, Moves::move m) {
	float modifier = 1;
	if (m.t == Moves::CPU_t) {
		if (target->type == Fighter::CPU_t) {
			modifier = 2;
			effective = 1;
		} else {
			modifier = 0.5;
			effective = -1;
		}
	} else if (m.t == Moves::GPU_t) {
		if (target->type == Fighter::GPU_t) {
			modifier = 2;
			effective = 1;
		} else {
			modifier = 0.5;
			effective = -1;
		}
	} else if (m.t == Moves::dual_t) {
		modifier = 1;
		effective = 0;
	}
	modifier *= map_val(random_float(), -1.0, 1.0, 0.85, 1.0);
	if (map_val(random_float(), -1, 1, 0, 1.0) < caster->stats.speed / 512.0f) {
		cout << caster->name << " had a crit hit!" << endl;
		modifier *= 1.5;
		effective = 3;
	}
	int damage = ((2.0f * caster->level / 5.0f + 2) * m.power * caster->stats.atk / (target->stats.def * 50.0f) + 2) * modifier;
	if (m.name == "glPolygonOffset") {
		if (target->name.find("Z-Buffer-Fighting") != string::npos) {
			damage *= 6.0f;
			effective = 2;
			cout << "zbf yes" << endl;
		} else {
			damage /= 2.0f;
			cout << "not zbf?" << endl;
		}
	}
	cout << caster->name << " did " << damage << " damage." << endl;
	return damage;
}

Moves::single_attack_t Moves::move_off_on(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target) {
	return default_attack(caster, target, "Off and On again");
}

Moves::single_attack_t Moves::move_arm_smash(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target) {
	return default_attack(caster, target, "Arm Smash");
}

Moves::single_attack_t Moves::move_defensive_stance(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target) {
	stat_diff_t diff;
	Fighter::stats_t s;
	s.atk = 0;
	s.def = 0;
	s.speed = 0;
	s.hp = 0;

	diff.second = s;
	float at_least = 0.3 * caster->level_stats.def;
	float at_most = 0.5 * caster->level_stats.def;
	float get = map_val(random_float(), -1, 1, at_least, at_most);
	if (caster->stats.def < 4 * caster->level_stats.def) {
		s.def = get;
	} else {
		s.def = 1;
	}
	diff.first = s;
	single_attack_t attack{"Defensive Stance", diff, 0};
	return attack;
}

Moves::single_attack_t Moves::move_body_slam(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target) {
	return default_attack(caster, target, "Body Slam");
}

//sl
Moves::single_attack_t Moves::move_plow(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target) {
	return default_attack(caster, target, "Plow");
}

Moves::single_attack_t Moves::move_terminal_velocity(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target) {
	return default_attack(caster, target, "Terminal Velocity");
}

Moves::single_attack_t Moves::move_smoke_bomb(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target) {
	stat_diff_t diff;
	Fighter::stats_t s;
	s.atk = 0;
	s.def = 0;
	s.speed = 0;
	s.hp = 0;

	diff.first = s;
	float at_least = 0.1 * target->stats.atk + 1;
	float at_most = 0.3 * target->stats.atk + 1;
	float get_atk = -map_val(random_float(), -1, 1, at_least, at_most);
	if (target->stats.atk > 0.3 * target->level_stats.atk) {
		s.atk = get_atk;
	} else {
		s.atk = -1;
	}
	diff.second = s;
	single_attack_t attack{"Smoke Bomb", diff, 0};
	return attack;
}

// zbf
Moves::single_attack_t Moves::move_fight_me(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target) {
	return default_attack(caster, target, "Fight Me");
}

Moves::single_attack_t Moves::move_z_buffs(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target) {
	stat_diff_t diff;
	Fighter::stats_t s;
	s.atk = 0;
	s.def = 0;
	s.speed = 0;
	s.hp = 0;

	diff.second = s;
	float at_least = 0.1 * ((caster->level_stats.atk + caster->level_stats.def + caster->level_stats.speed) / 3.0) + 1;
	float at_most = 0.3 * ((caster->level_stats.atk + caster->level_stats.def + caster->level_stats.speed) / 3.0) + 1;
	float get_atk = map_val(random_float(), -1, 1, at_least, at_most);
	float get_def = map_val(random_float(), -1, 1, at_least, at_most);
	float get_spd = map_val(random_float(), -1, 1, at_least, at_most);
	if (caster->stats.def < 4 * caster->level_stats.def) {
		s.def = get_def;
	} else {
		s.def = 1;
	}
	if (caster->stats.atk < 4 * caster->level_stats.atk) {
		s.atk = get_atk;
	} else {
		s.atk = 1;
	}
	if (caster->stats.speed < 4 * caster->level_stats.speed) {
		s.speed = get_spd;
	} else {
		s.speed = 1;
	}
	diff.first = s;
	single_attack_t attack{"Z-Buffs", diff, 0};
	return attack;
}

Moves::single_attack_t Moves::move_wall_of_ugly(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target) {
	return default_attack(caster, target, "Wall of Ugly");
}

Moves::single_attack_t Moves::move_default(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target) {
	return default_attack(caster, target, "Back to Basics");
}

Moves::single_attack_t Moves::default_attack(std::shared_ptr<Fighter> caster, std::shared_ptr<Fighter> target, string move_name) {
	float damage = calc_damage(caster, target, all_moves[move_name]);
	stat_diff_t diff;
	Fighter::stats_t s;
	s.atk = 0;
	s.def = 0;
	s.speed = 0;
	s.hp = 0;

	diff.first = s;
	s.hp = -glm::clamp(damage, 1.0f, 512.0f);
	diff.second = s;
	single_attack_t attack{move_name, diff, effective};
	return attack;
}

