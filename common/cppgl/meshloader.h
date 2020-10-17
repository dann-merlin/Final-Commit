#pragma once

#include <map>
#include <string>
#include <memory>
#include <functional>
#include "mesh.h"
//#include "cppgl/animation/animatedmesh.h"
#include "material.h"
#include "shader.h"
#include "drawelement.h"

// if normalize == true: translate mesh to origin and scale vertices to fit [-1, 1]
// use the set_shader_callback to set shaders depending on the material (f.e. with/-out normalmap)

class MeshLoader {
public:
	static std::vector<std::shared_ptr<Drawelement>> load(const std::string &filename, bool normalize = false,
														  std::function<std::shared_ptr<Shader>(const std::shared_ptr<Material> &)> set_shader_callback =
														  [](const std::shared_ptr<Material> &) { return std::shared_ptr<Shader>(); });

	static std::vector<std::shared_ptr<Drawelement>> load_animated(const std::string &filename, std::string animation_name, glm::mat4 default_trafo = glm::mat4(1),
																   std::function<std::shared_ptr<Shader>(const std::shared_ptr<Material> &)> set_shader_callback = [](
																		   const std::shared_ptr<Material> &) { return std::shared_ptr<Shader>(); });

	// data
	static std::string base_path;

	//structs
	struct Bone {
		std::string name;
		glm::mat4 offset;
		glm::mat4 transform;
	};

	static void load_animation(const std::string &filename, std::string animation_name, const std::vector<std::shared_ptr<Drawelement>> &animated_prototype,  bool default_attack = false);
};
