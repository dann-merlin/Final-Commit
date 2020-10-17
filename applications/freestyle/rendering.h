#pragma once

#include <cppgl/shader.h>
#include <chrono>
#include <cppgl/texture.h>
#include <cppgl/framebuffer.h>

// draw random floats in [-1, 1]
inline float random_float() {
//	std::tm *now = std::localtime(&t);
	srand((uint) std::chrono::system_clock::now().time_since_epoch().count());
	return (float(rand() % 32768) / 16384.0f) - 1.0f;
}

inline glm::vec2 random_vec2() { return glm::vec2(random_float(), random_float()); }

inline glm::vec3 random_vec3() { return glm::vec3(random_float(), random_float(), random_float()); }

inline float map_val(float input, float input_start, float input_end, float output_start, float output_end) {
	return output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start);
}

void setup_light(const std::shared_ptr<Shader> &shader);

void update_loading_screen();

void deferred_shading_pass(const std::shared_ptr<Framebuffer> &gbuffer, const std::shared_ptr<Framebuffer> &gbuffer_outline);

void deferred_world_pass(const std::shared_ptr<Framebuffer> &gbuffer, glm::vec3 player_pos = glm::vec3(0));

void draw_shader(std::shared_ptr<Shader> shader, float time_p = -1.f, std::shared_ptr<Texture2D> tex = 0);

void draw_blur(std::shared_ptr<Shader> shader, std::shared_ptr<Framebuffer> blur_buffer, float blur_strength);

void deferred_retro_pass(const std::shared_ptr<Framebuffer> &gbuffer, const std::shared_ptr<Framebuffer> &gbuffer_outline);

void blit(const Texture2D &tex);

void blit(const std::shared_ptr<Texture2D> &tex);

void blit_alpha(const Texture2D &tex);

void blit_alpha(const std::shared_ptr<Texture2D> &tex);
