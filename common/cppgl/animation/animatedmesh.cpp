#include "animatedmesh.h"
#include <iostream>
#include <string>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

inline uint32_t type_to_bytes(GLenum type) {
	switch (type) {
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			return 1;
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
		case GL_HALF_FLOAT:
			return 2;
		case GL_FLOAT:
		case GL_FIXED:
		case GL_INT:
		case GL_UNSIGNED_INT:
			return 4;
		case GL_DOUBLE:
			return 8;
		default:
			throw std::runtime_error("Unknown GL type!");
	}
}

inline glm::vec3 to_glm(const aiVector3D &v) { return glm::vec3(v.x, v.y, v.z); }

inline glm::quat to_glm(const aiQuaternion &q) { return glm::quat(q.w, q.x, q.y, q.z); }

inline glm::mat4 to_glm(const aiMatrix4x4 &m) {
	return glm::mat4(m.a1, m.b1, m.c1, m.d1, m.a2, m.b2, m.c2, m.d2,
					 m.a3, m.b3, m.c3, m.d3, m.a4, m.b4, m.c4, m.d4);
//	return glm::mat4(m.a1, m.a2, m.a3, m.a4, m.b1, m.b2, m.b3, m.b4,
//					 m.c1, m.c2, m.c3, m.c4, m.d1, m.d2, m.d3, m.d4);
}

// AnimatedMesh class

AnimatedMesh::AnimatedMesh(const std::string &name) : Mesh(name) {

}

AnimatedMesh::AnimatedMesh(const std::string &name, std::string animation_name, const aiMesh *mesh_ai, const aiScene *scene_ai) : AnimatedMesh(name) {
	// extract vertices, normals and texture coords
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;
	positions.reserve(mesh_ai->mNumVertices);
	normals.reserve(mesh_ai->HasNormals() ? mesh_ai->mNumVertices : 0);
	texcoords.reserve(mesh_ai->HasTextureCoords(0) ? mesh_ai->mNumVertices : 0);
	for (uint32_t i = 0; i < mesh_ai->mNumVertices; ++i) {
		positions.emplace_back(to_glm(mesh_ai->mVertices[i]));
		if (mesh_ai->HasNormals())
			normals.emplace_back(to_glm(mesh_ai->mNormals[i]));
		if (mesh_ai->HasTextureCoords(0))
			texcoords.emplace_back(glm::vec2(to_glm(mesh_ai->mTextureCoords[0][i])));
	}

	// extract faces
	std::vector<uint32_t> indices;
	indices.reserve(mesh_ai->mNumFaces * 3);
	for (uint32_t i = 0; i < mesh_ai->mNumFaces; ++i) {
		const aiFace &face = mesh_ai->mFaces[i];
		if (face.mNumIndices == 3) {
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		} else
			std::cout << "WARN: AnimatedMesh: skipping non-triangle face!" << std::endl;
	}

	// extract bones and bone weights

//	std::vector<std::shared_ptr<Bone>> bones;
	std::vector<std::vector<uint>> bone_indices;
	std::vector<std::vector<float>> bone_weights;
	for (uint j = 0; j < mesh_ai->mNumVertices; j++) {
		std::vector<uint> bi;
		bi.reserve(4);
		bone_indices.push_back(bi);
		std::vector<float> bw;
		bw.reserve(4);
		bone_weights.push_back(bw);
	}
	for (uint b = 0; b < mesh_ai->mNumBones; b++) {
		aiBone *ai_bone = mesh_ai->mBones[b];
		for (uint w = 0; w < ai_bone->mNumWeights; w++) {
			auto weight = ai_bone->mWeights[w];
			int vertex = weight.mVertexId;
			bone_indices[vertex].push_back(b);
			bone_weights[vertex].push_back(weight.mWeight);
		}
		auto bone = std::make_shared<Bone>(b, ai_bone->mName.data, to_glm(ai_bone->mOffsetMatrix));
		bones.push_back(bone);
	}
	std::vector<glm::ivec4> bone_indices_vertex;
	std::vector<glm::vec4> bone_weights_vertex;
	for (uint j = 0; j < mesh_ai->mNumVertices; j++) {
		uint b0, b1, b2, b3;
		b0 = bone_indices[j].size() > 0 ? bone_indices[j][0] : -1;
		b1 = bone_indices[j].size() > 1 ? bone_indices[j][1] : -1;
		b2 = bone_indices[j].size() > 2 ? bone_indices[j][2] : -1;
		b3 = bone_indices[j].size() > 3 ? bone_indices[j][3] : -1;
		bone_indices_vertex.emplace_back(glm::ivec4(b0, b1, b2, b3));
		float w0, w1, w2, w3;
		w0 = bone_weights[j].size() > 0 ? bone_weights[j][0] : 0;
		w1 = bone_weights[j].size() > 1 ? bone_weights[j][1] : 0;
		w2 = bone_weights[j].size() > 2 ? bone_weights[j][2] : 0;
		w3 = bone_weights[j].size() > 3 ? bone_weights[j][3] : 0;
		bone_weights_vertex.emplace_back(glm::vec4(w0, w1, w2, w3));
	}

	// get root bone
	const aiNode *root_node = scene_ai->mRootNode;
	std::string node_name = root_node->mName.data;
	int curr_bone_id = 0;
	for (const auto &b : bones) {
		if (b->name == node_name) {
			curr_bone_id = b->id;
		}
	}
	root_bone = bones[curr_bone_id];
	glm::mat4 inverse_root_trafo = to_glm(scene_ai->mRootNode->mTransformation);
	Bone::global_inverse_trafo = inverse_root_trafo;

	// extract animations
	load_animation(scene_ai, animation_name, true);


	// build GL data
	add_vertex_buffer(GL_FLOAT, 3, positions.size(), positions.data());
	if (not normals.empty())
		add_vertex_buffer(GL_FLOAT, 3, normals.size(), normals.data());
	if (not texcoords.empty())
		add_vertex_buffer(GL_FLOAT, 2, texcoords.size(), texcoords.data());
	add_vertex_buffer(GL_INT, 4, bone_indices_vertex.size(), bone_indices_vertex.data());
	add_vertex_buffer(GL_FLOAT, 4, bone_weights_vertex.size(), bone_weights_vertex.data());
	add_index_buffer(indices.size(), indices.data());
}

AnimatedMesh::AnimatedMesh(const std::string &name,
						   const std::shared_ptr<Bone> &root_bone_in,
						   const int bone_count_in,
						   const std::vector<glm::vec3> &positions,
						   const std::vector<uint32_t> &indices,
						   const std::vector<glm::vec3> &normals,
						   const std::vector<glm::vec2> &texcoords,
						   const std::vector<glm::ivec4> bone_indices,
						   const std::vector<glm::vec4> bone_weights) : AnimatedMesh(name) {
	// build GL data
	root_bone = root_bone_in;
	bone_count = bone_count_in;
	add_vertex_buffer(GL_FLOAT, 3, positions.size(), positions.data());
	if (not normals.empty())
		add_vertex_buffer(GL_FLOAT, 3, normals.size(), normals.data());
	if (not texcoords.empty())
		add_vertex_buffer(GL_FLOAT, 2, texcoords.size(), texcoords.data());
	if (not bone_indices.empty())
		add_vertex_buffer(GL_INT, 4, bone_indices.size(), bone_indices.data());
	if (not bone_weights.empty())
		add_vertex_buffer(GL_FLOAT, 4, bone_weights.size(), bone_weights.data());
	add_index_buffer(indices.size(), indices.data());
}

AnimatedMesh::~AnimatedMesh() {
}

void AnimatedMesh::do_animation(std::string anim) {
	animator.do_animation(anim);
}

void AnimatedMesh::update() {
	animator.update(root_bone);
}

std::vector<glm::mat4> AnimatedMesh::get_joint_transforms() {
	std::vector<glm::mat4> trafos;
	add_bone_trafos(root_bone, trafos);
	return trafos;
}

void AnimatedMesh::add_bone_trafos(std::shared_ptr<Bone> bone, std::vector<glm::mat4> &trafos) {
	trafos.push_back(root_bone->get_animated_trafo());
	for (auto &child : bone->children) {
		add_bone_trafos(child.second, trafos);
	}
}

void AnimatedMesh::draw(const std::shared_ptr<Shader> &shader) const {
	std::vector<glm::mat4> bone_trafos;
	for (uint i = 0; i < bones.size(); i++) {
		glm::mat4 animated_trafo = bones[i]->get_animated_trafo();
		if (animated_trafo == glm::mat4(0)) {
			std::cerr << "AnimatedMesh: animated_trafo was mat4(0). This should not have happened!" << std::endl;
			animated_trafo = glm::mat4(1);
			bones[i]->set_animated_trafo(glm::mat4(1));
		}
		bone_trafos.push_back(animated_trafo);
//		std::cout << bones[i]->name << std::endl;
//		std::cout << "animated " << i << ":" << glm::to_string(bone_trafos[i]) << std::endl;
//		std::cout << " loc_def " << i << ":" << glm::to_string(bones[i]->local_default_trafo) << std::endl;
	}
	shader->uniform("bone_transforms", bone_trafos.data(), bone_trafos.size());
	if (ibo)
		glDrawElements(primitive_type, num_indices, GL_UNSIGNED_INT, 0);
	else
		glDrawArrays(primitive_type, 0, num_vertices);
}

int AnimatedMesh::read_node_heirarchy(std::shared_ptr<Animation> &animation, const aiAnimation *animation_ai, const aiNode *parent_node, glm::mat4 parent_trafo, bool first) {
	std::string node_name = parent_node->mName.data;
	glm::mat4 node_trafo = to_glm(parent_node->mTransformation);
	aiNodeAnim *p_node_anim = NULL;
	for (uint i = 0; i < animation_ai->mNumChannels; i++) {
		auto tmp_node_anim = animation_ai->mChannels[i];
		if (std::string(tmp_node_anim->mNodeName.data) == node_name) {
			p_node_anim = tmp_node_anim;
			break;
		}
	}
	int curr_bone_id = -1;
	for (const auto &b : bones) {
		if (b->name == node_name) {
			curr_bone_id = b->id;
		}
	}
	std::vector<std::shared_ptr<KeyFrame>> frames;
	std::shared_ptr<BoneAnimation> bone_anim;
	if (p_node_anim) {
		if (p_node_anim->mNumPositionKeys != p_node_anim->mNumRotationKeys || p_node_anim->mNumPositionKeys != p_node_anim->mNumScalingKeys) {
			std::cerr << "Please always set LocRotScale" << std::endl;
			exit(1);
		}
		for (uint i = 0; i < p_node_anim->mNumPositionKeys; i++) {
			auto loc = p_node_anim->mPositionKeys[i];
			auto rot = p_node_anim->mRotationKeys[i];
			auto scale = p_node_anim->mScalingKeys[i];

//			std::cout << rot.mValue.w << " " << rot.mValue.x << " " << rot.mValue.y << " " << rot.mValue.z << std::endl;
			BoneTransform b(to_glm(loc.mValue), to_glm(rot.mValue), to_glm(scale.mValue));
			auto k = std::make_shared<KeyFrame>(loc.mTime, b);
			frames.push_back(k);
		}
		bone_anim = std::make_shared<BoneAnimation>(curr_bone_id, animation_ai->mDuration, frames);
	} else {
		BoneTransform b(glm::vec3(node_trafo[3]), glm::quat_cast(glm::mat3(node_trafo)), glm::vec3(1));
		auto k = std::make_shared<KeyFrame>(0, b);
		auto k2 = std::make_shared<KeyFrame>(animation->length, b);
		frames.push_back(k);
		frames.push_back(k2);
		if (curr_bone_id != -1 && animation->bone_animations.count(curr_bone_id) == 0 && node_name == bones[curr_bone_id]->name) {
			std::cout << curr_bone_id << " " << node_name << " is using " << glm::to_string(node_trafo) << std::endl;
			bone_anim = std::make_shared<BoneAnimation>(curr_bone_id, animation_ai->mDuration, frames);
		}
	}
	if (curr_bone_id != -1) {
		animation->bone_animations[curr_bone_id] = bone_anim;
	}
	for (uint i = 0; i < parent_node->mNumChildren; i++) {
		int child_bone = read_node_heirarchy(animation, animation_ai, parent_node->mChildren[i], glm::mat4(1), first);
		if (curr_bone_id == -1) {
			curr_bone_id = child_bone;
		} else {
			if (child_bone != -1) {
				bones[curr_bone_id]->add_child(bones[child_bone]);
			}
		}
	}
	return curr_bone_id;
}

void AnimatedMesh::load_animation(const aiScene *scene_ai, std::string animation_name, bool first,  bool default_attack) {
	if (scene_ai->mNumAnimations == 0) {
		std::cerr << "There were no animations in file!" << std::endl;
		return;
	}
	auto animation_ai = scene_ai->mAnimations[0];
	auto animation = std::make_shared<Animation>(0, animation_ai->mName.data, animation_ai->mDuration);
	std::cout << "Loading animation: " << animation_ai->mName.data << std::endl;
	const aiNode *root_node = scene_ai->mRootNode;
	std::string node_name = root_node->mName.data;
//	int curr_bone_id = -1;
//	for (const auto &b : bones) {
//		if (b->name == node_name) {
//			curr_bone_id = b->id;
//		}
//	}

	int root_id = read_node_heirarchy(animation, animation_ai, root_node, glm::mat4(1), first);
	std::cout << "root_id: " << root_id << std::endl;
	root_bone = bones[root_id];
	animator.animations[animation_name] = animation;
	if(default_attack)
	animator.animations["default_attack"] = animation;

	for (const auto &b : bones) {
		if (animation->bone_animations.count(b->id) == 0) {
			std::cerr << "There was no bone animation set for bone " << b->id << " " << b->name << std::endl;
		}
	}

	std::cout << "num animations: " << animator.animations.size() << std::endl;
	for (auto &anim : animator.animations) {
		std::cout << "\tbone animations: " << anim.second->bone_animations.size() << std::endl;
		for (auto &ba : anim.second->bone_animations) {
			std::cout << "\t\tid: " << ba.second->bone_id << ", keyframes: " << ba.second->frames.size() << std::endl;
		}
	}
	animator.current_animation = animation_name;
}


// ------------------------------------------
// AnimatedMesh class
