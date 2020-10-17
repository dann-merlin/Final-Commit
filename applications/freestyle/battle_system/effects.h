#ifndef CPPGL_EFFECTS_H
#define CPPGL_EFFECTS_H

#include <unordered_map>
#include <cppgl/drawelement.h>

class Effects {
public:
	static std::unordered_map<std::string, std::vector<std::shared_ptr<Drawelement>>> effects;
	static std::vector<std::shared_ptr<Drawelement>> current_effect;
	static std::unordered_map<std::string, std::shared_ptr<Shader>> effect_shader;
	static std::shared_ptr<Shader> current_effect_shader;
	static glm::mat4 trafo;
	static bool loaded;
	static float time_hot_fix;

	static void init_effects();

	static void play_effect(std::string effect_name = "idle", glm::mat4 trafo_new = glm::mat4(1));

	static void draw(const std::shared_ptr<Shader> &shader = NULL);
};


#endif //CPPGL_EFFECTS_H
