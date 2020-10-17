#ifndef CPPGL_ANIMATEDMESH_H
#define CPPGL_ANIMATEDMESH_H

#include <cppgl/mesh.h>
#include <cppgl/shader.h>
#include <assimp/scene.h>
#include "./animator.h"
#include "./bone.h"

class AnimatedMesh : public Mesh {
public:
	// data
	Animator animator;
	std::shared_ptr<Bone> root_bone;
	std::vector<std::shared_ptr<Bone>> bones;
	int bone_count;


	// construct empty mesh
	AnimatedMesh(const std::string &name);

	// construct from assimp mesh
	AnimatedMesh(const std::string &name, std::string animation_name, const aiMesh *mesh_ai, const aiScene *scene_ai);

	// construct from triangle soup
	AnimatedMesh(const std::string &name,
				 const std::shared_ptr<Bone> &root_bone_in,
				 const int bone_count_in,
				 const std::vector<glm::vec3> &positions,
				 const std::vector<uint32_t> &indices,
				 const std::vector<glm::vec3> &normals = std::vector<glm::vec3>(),
				 const std::vector<glm::vec2> &texcoords = std::vector<glm::vec2>(),
				 const std::vector<glm::ivec4> bone_indices = std::vector<glm::ivec4>(),
				 const std::vector<glm::vec4> bone_weights = std::vector<glm::vec4>());

	virtual ~AnimatedMesh();

	// prevent copies and moves, since GL buffers aren't reference counted
	AnimatedMesh(const AnimatedMesh &) = delete;

	AnimatedMesh &operator=(const AnimatedMesh &) = delete;

	AnimatedMesh &operator=(const AnimatedMesh &&) = delete;

	void do_animation(std::string anim);

	void update();

	std::vector<glm::mat4> get_joint_transforms();

	void add_bone_trafos(std::shared_ptr<Bone> bone, std::vector<glm::mat4> &vector);

	void draw(const std::shared_ptr<Shader> &shader) const;

	void load_animation(const aiScene *scene_ai, std::string animation_name, bool first = false, bool default_attack = false);

	int read_node_heirarchy(std::shared_ptr<Animation> &animation, const aiAnimation *animation_ai, const aiNode *parent_node, glm::mat4 parent_trafo, bool first);
};

// variadic alias for std::make_shared<>(...)
template<class... Args>
std::shared_ptr<AnimatedMesh> make_animated_mesh(Args &&... args) {
	return std::make_shared<AnimatedMesh>(args...);
}


#endif //CPPGL_ANIMATEDMESH_H
