#include <cppgl/cppgl.h>
#include <cppgl/shader.h>
#include "soup.h"
#include "rendering.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui/imgui.h>

std::shared_ptr<Drawelement> Soup::prototype;
std::shared_ptr<Framebuffer> Soup::fb;
std::shared_ptr<Shader> Soup::update_soup_shader;
int Soup::w = 100, Soup::h = 100;

using namespace std;
using namespace glm;

Soup::Soup(float height) : height(height), pos(0,height,0) {
	init_soup();
}

void Soup::init_soup() {
	if (prototype) return;
	prototype = make_drawelement("soup_prototype");
	int resol = 4000;
	fb = make_framebuffer("soup_fb", resol, resol);
	update_soup_shader = make_shader("update_soup_shader", "shader/soup_update.vs", "shader/soup_update.fs");
	fb->attach_depthbuffer(make_texture("soup_depth", resol, resol, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT));
	fb->attach_colorbuffer(make_texture("soup_texture", resol, resol, GL_RGBA32F, GL_RGBA, GL_FLOAT));
	fb->check();
	// setup mesh
	vector<vec3> vertices = {{-1, 0, -1},
							 {-1, 0, 1},
							 {1, 0, 1},
							 {1, 0, -1}};
	unsigned int indices[6] = {0,1,2,0,2,3};
	auto mesh = make_mesh("soup_mesh");
	mesh->set_primitive_type(GL_TRIANGLES);
	mesh->add_vertex_buffer(GL_FLOAT, 3, vertices.size(), vertices.data(), GL_STATIC_DRAW);
	mesh->add_index_buffer(6, indices);
	prototype->add_mesh(mesh);
	// setup material
	auto mat = make_material("soup_material");
	prototype->use_material(mat);
	// setup shader
	auto shader = make_shader("soup_shader", "shader/soup_apply.vs", "shader/soup_apply.fs");
	prototype->use_shader(shader);
}

void Soup::update() {
	static bool active = true;
	// ImGui::Begin("Soup");
	// ImGui::Checkbox("activate rising soup", &active);
	// ImGui::End();
	if(active) pos += vec3(0,Context::frame_time() * rising_speed / 1000.0f,0);
	glm::mat4 translation_mat = translate(pos);
	glm::mat4 scale_mat = scale(vec3(1000));
	trafo = translation_mat * scale_mat * mat4(1);
}

void Soup::update_soup_texture() {
	fb->bind();
	// static float time_matrix = 0;
	// time_matrix += Context::frame_time();
	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// glDisable(GL_DEPTH_TEST);
	// update_soup_shader->bind();
	// update_soup_shader->uniform("iTime", (float) time_matrix/ 1000);
	// update_soup_shader->uniform("iResolution", Context::resolution());
	// update_soup_shader->unbind();
	// Quad::draw();
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, fb->color_textures[0]->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	draw_shader(update_soup_shader);

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	fb->unbind();
}

void Soup::draw() {
	prototype->material->add_texture("tex", fb->color_textures[0]);
	glEnable(GL_BLEND);
	// glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	prototype->bind();
	setup_light(prototype->shader);
	prototype->draw(trafo);
	prototype->unbind();
	glDisable(GL_BLEND);
}

// void Soup::draw() {
// 	static float time_matrix = 0;
// 	time_matrix += Context::frame_time();
// 	glEnable(GL_BLEND);
// 	glDepthMask(GL_FALSE);
// 	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
// 	prototype->bind();
// 	prototype->shader->uniform("iTime", (float) time_matrix/ 1000);
// 	prototype->shader->uniform("iResolution", Context::resolution());
// 	prototype->shader->uniform("player_pos", Camera::current()->pos);
// 	setup_light(prototype->shader);
// 	prototype->draw(trafo);
// 	prototype->unbind();
// 	glDisable(GL_BLEND);
// 	glDepthMask(GL_TRUE);
// }
