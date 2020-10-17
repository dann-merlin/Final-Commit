#include "rendering.h"
#include <cppgl/quad.h>
#include <cppgl/camera.h>
#include <cppgl/context.h>
#include <cppgl/texture.h>
#include <iostream>
#include "../../applications/freestyle/gui.h"

using namespace std;

extern std::shared_ptr<Framebuffer> gbuffer;

void setup_light(const std::shared_ptr<Shader> &shader) {
	shader->uniform("ambient_col", glm::vec3(0.12f, 0.14f, 0.16f));
	shader->uniform("light_dir", glm::normalize(glm::vec3(1.f, -0.6f, -0.4f)));
	shader->uniform("light_col", glm::vec3(0.6f, 0.7f, 0.8f));
	shader->uniform("cam_pos", Camera::current()->pos);
}

void update_loading_screen() {
	static uint counted_calls = 0;
	/*cout << "this is counted call number: " << */counted_calls++/* << endl*/;
	glClear(GL_COLOR_BUFFER_BIT);
	GUI::draw_loading_screen((float) counted_calls / 69.f, "Loading render data... please be patient...");
	Context::swap_buffers();
}

void deferred_shading_pass(const std::shared_ptr<Framebuffer> &gbuffer, const std::shared_ptr<Framebuffer> &gbuffer_outline) {
	static auto shader = make_shader("deferred-lighting", "shader/copytex.vs", "shader/deferred_lighting.fs");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->bind();
	setup_light(shader);
	shader->uniform("gbuf_depth", gbuffer->depth_texture, 0);
	shader->uniform("gbuf_diff", gbuffer->color_textures[0], 1);
	shader->uniform("gbuf_pos", gbuffer->color_textures[1], 2);
	shader->uniform("gbuf_norm", gbuffer->color_textures[2], 3);
	shader->uniform("gbuf_outline", gbuffer_outline->color_textures[0], 4);
	shader->uniform("near_far", glm::vec2(Camera::current()->near, Camera::current()->far));
	shader->uniform("cam_pos", Camera::current()->pos);
	shader->uniform("screenres", glm::vec2(Context::resolution()));
	Quad::draw();
	shader->unbind();
}

void draw_blur(std::shared_ptr<Shader> shader, std::shared_ptr<Framebuffer> blur_buffer, float blur_strength)  {
	shader->bind();
	glDisable(GL_DEPTH_TEST);
	setup_light(shader);
	shader->uniform("blur_buff", blur_buffer->color_textures[0], 0);
	shader->uniform("velocity", blur_strength);
	Quad::draw();
	glEnable(GL_DEPTH_TEST);
	shader->unbind();
}

void draw_shader(std::shared_ptr<Shader> shader, float time_p, std::shared_ptr<Texture2D> tex) {
	static float time_matrix = 0;
	// if(!tex) tex = make_texture("TextureMatrix", "render-data/images/alex.png");
	time_matrix += Context::frame_time();
	shader->bind();
	glDisable(GL_DEPTH_TEST);
	// if(blend) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// }
	shader->uniform("iTime",time_p == -1.f ? (float) time_matrix/2000 : time_p);
	shader->uniform("iResolution", Context::resolution());
	glm::ivec2 mp = Context::mouse_pos();
	shader->uniform("iMouse", glm::vec4(mp.x, mp.y, 0,0));
	shader->uniform("cam_pos", Camera::current()->pos);
	// shader->uniform("soup_height", the_soup->height);
	// std::cout << mp.x << std::endl;
	if(tex) shader->uniform("iChannel0", *tex, 0);
	Quad::draw();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	shader->unbind();
}

void deferred_world_pass(const std::shared_ptr<Framebuffer> &gbuffer, glm::vec3 player_pos) {
	static auto shader = make_shader("deferred-normals", "shader/copytex.vs", "shader/deferred_world.fs");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->bind();
	setup_light(shader);
	shader->uniform("gbuf_depth", gbuffer->depth_texture, 4);
	shader->uniform("gbuf_diff", gbuffer->color_textures[0], 5);
	shader->uniform("gbuf_pos", gbuffer->color_textures[1], 6);
	shader->uniform("gbuf_norm", gbuffer->color_textures[2], 7);
	shader->uniform("near_far", glm::vec2(Camera::current()->near, Camera::current()->far));
	shader->uniform("cam_pos", Camera::current()->pos);
	shader->uniform("player_pos", player_pos);
	shader->uniform("screenres", glm::vec2(Context::resolution()));
	Quad::draw();
	shader->unbind();
}

void deferred_retro_pass(const std::shared_ptr<Framebuffer> &gbuffer, const std::shared_ptr<Framebuffer> &gbuffer_outline) {
	static auto shader = make_shader("deferred-toon", "shader/copytex.vs", "shader/deferred_toon.fs");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->bind();
	setup_light(shader);
	shader->uniform("gbuf_depth", gbuffer->depth_texture, 0);
	shader->uniform("gbuf_diff", gbuffer->color_textures[0], 1);
	shader->uniform("gbuf_pos", gbuffer->color_textures[1], 2);
	shader->uniform("gbuf_norm", gbuffer->color_textures[2], 3);
	shader->uniform("gbuf_outline", gbuffer_outline->color_textures[0], 4);
	shader->uniform("near_far", glm::vec2(Camera::current()->near, Camera::current()->far));
	shader->uniform("cam_pos", Camera::current()->pos);
	shader->uniform("screenres", glm::vec2(Context::resolution()));
	Quad::draw();
	shader->unbind();
}

void blit(const std::shared_ptr<Texture2D> &tex) {
	blit(*tex);
}

void blit(const Texture2D &tex) {
	static auto shader = make_shader("blit", "shader/copytex.vs", "shader/copytex.fs");
	shader->bind();
	shader->uniform("tex", tex, 0);
	glDisable(GL_DEPTH_TEST);
	Quad::draw();
	glEnable(GL_DEPTH_TEST);
	shader->unbind();
}

void blit_alpha(const std::shared_ptr<Texture2D> &tex) {
	blit_alpha(*tex);
}

void blit_alpha(const Texture2D &tex) {
	static auto shader = make_shader("blit", "shader/copytex_alpha.vs", "shader/copytex_alpha.fs");
	shader->bind();
	shader->uniform("tex", tex, 0);
	shader->uniform("screenres", glm::vec2(Context::resolution()));
	if(gbuffer){
		shader->uniform("rendered_diff", gbuffer->color_textures[0], 1);
	}
	glDisable(GL_DEPTH_TEST);
	Quad::draw();
	glEnable(GL_DEPTH_TEST);
	shader->unbind();
}
