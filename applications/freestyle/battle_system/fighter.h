#ifndef CPPGL_FIGHTER_H
#define CPPGL_FIGHTER_H

#include <cppgl/drawelement.h>

class Fighter {
public:

	std::vector<std::shared_ptr<Drawelement>> prototype;
	enum fighter_type {
		CPU_t, GPU_t, dual_t
	};
	struct stats_t {
		int atk = 1;
		int def = 1;
		int speed = 1;
		int hp = 10;
	};
	fighter_type type = dual_t;
	stats_t base_stats, stats, level_stats;
	int level = 1;
	int max_hp = 10;
	std::string name;
	std::vector<std::string> moves, future_moves;
	glm::mat4 trafo;
	std::string anim_id = "no_anim";
	std::string last_move = "no_move";

	Fighter();

	Fighter(glm::mat4 t, const std::vector<std::shared_ptr<Drawelement>> &proto);

	void draw(std::shared_ptr<Shader> shader, float time = -1, bool wire = false);

	void do_animation(std::string anim_id);

	std::string get_random_move();

	void apply_stat_diff(stats_t diff);

	void set_base_stats(int hp, int atk, int def, int speed);

	void calculate_stats();

	void level_up(int new_level);

	void update();
};


#endif //CPPGL_FIGHTER_H
