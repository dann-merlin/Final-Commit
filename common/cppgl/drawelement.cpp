#include "drawelement.h"
#include "camera.h"
#include <iostream>

using namespace std;

Drawelement::Drawelement(const std::string &name) : NamedMap(name) {}

Drawelement::Drawelement(const std::string &name, const std::shared_ptr<Shader> &shader, const std::shared_ptr<Material> &material)
		: NamedMap(name), shader(shader), material(material) {}

Drawelement::Drawelement(const std::string &name, const std::shared_ptr<Shader> &shader, const std::shared_ptr<Material> &material,
						 const std::shared_ptr<Mesh> &mesh) : NamedMap(name), shader(shader), material(material) {
	add_mesh(mesh);
}

Drawelement::Drawelement(const std::string &name, const std::shared_ptr<Shader> &shader, const std::shared_ptr<Material> &material,
						 const std::vector<std::shared_ptr<Mesh>> &meshes) : NamedMap(name), shader(shader), material(material), meshes(meshes) {}

Drawelement::~Drawelement() {}

void Drawelement::bind() const {
	if (shader) {
		shader->bind();
		if (material) material->bind(shader);
		shader->uniform("view", Camera::current()->view);
		shader->uniform("view_normal", Camera::current()->view_normal);
		shader->uniform("proj", Camera::current()->proj);
	}
}

void Drawelement::unbind() const {
	if (material) material->unbind();
	if (shader) shader->unbind();
}

void Drawelement::update() {
	for (const auto &mesh : meshes) {
		mesh->update();
	}
}

void Drawelement::draw(const glm::mat4 &model) const {
	if (shader) {
		shader->uniform("model", model);
		shader->uniform("model_normal", glm::transpose(glm::inverse(model)));
		shader->uniform("default_trafo", default_trafo);
	}
	for (const auto &mesh : meshes) {
		mesh->bind();
		mesh->draw(shader);
		mesh->unbind();
	}
}

void Drawelement::do_animation(std::string animation_id) {
	for (const auto &mesh : meshes) {
		mesh->do_animation(animation_id);
	}
}
