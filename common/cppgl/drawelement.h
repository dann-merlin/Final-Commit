#pragma once

#include "named_map.h"
#include "mesh.h"
#include "material.h"

// -----------------------------------------------
// Drawelement

class Drawelement : public NamedMap<Drawelement> {
public:
	Drawelement(const std::string& name);
	Drawelement(const std::string& name, const std::shared_ptr<Shader>& shader, const std::shared_ptr<Material>& material);
	Drawelement(const std::string& name, const std::shared_ptr<Shader>& shader, const std::shared_ptr<Material>& material,
			const std::shared_ptr<Mesh>& mesh);
	Drawelement(const std::string& name, const std::shared_ptr<Shader>& shader, const std::shared_ptr<Material>& material,
			const std::vector<std::shared_ptr<Mesh>>& meshes);
	virtual ~Drawelement();

	void do_animation(std::string animation_id);

	void bind() const;
	void unbind() const;

	void update();

	void draw(const glm::mat4& model) const;

	inline void add_mesh(const std::shared_ptr<Mesh>& mesh) { meshes.push_back(mesh); }
	inline void add_meshes(const std::vector<std::shared_ptr<Mesh>>& meshes) { for (auto& mesh : meshes) add_mesh(mesh); }
	inline void use_shader(const std::shared_ptr<Shader>& shader) { this->shader = shader; }
	inline void use_material(const std::shared_ptr<Material>& material) { this->material = material; }

	std::shared_ptr<Shader> shader;
	std::shared_ptr<Material> material;
	std::vector<std::shared_ptr<Mesh>> meshes;
	glm::mat4 default_trafo = glm::mat4(1);
};

// variadic alias for std::make_shared<>(...)
template <class... Args> std::shared_ptr<Drawelement> make_drawelement(Args&&... args) {
	return std::make_shared<Drawelement>(args...);
}
