#include "rope.h"
#include <imgui/imgui.h>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext.hpp>

#define ROPE_SEGMENTS 8
#define ROPE_VERTICES 99
#define ROPE_SIDES 10
#define RADIUS 0.1f

using namespace std;
using namespace glm;

extern float global_time;

inline vec3 to_glm(btVector3 &v) {
	return vec3(v.x(), v.y(), v.z());
}

inline vec3 to_glm(Vector v) {
	return vec3(v.x, v.y, v.z);
}

inline btVector3 to_bt(vec3 &v) {
	return btVector3(v.x, v.y, v.z);
}

inline Vector to_Vector(vec3 v) {
	return Vector(v.x, v.y, v.z);
}


Rope::Rope() {
	prototype = make_drawelement("rope");
	auto mesh = make_mesh("rope_mesh");
	mesh->set_primitive_type(GL_TRIANGLE_STRIP);

	vector<float> rope_verts_number;
	for (int i = 0; i < ROPE_VERTICES * ROPE_SIDES; i++) {
		rope_verts.push_back(vec3(i, i, i));
		rope_verts_number.push_back(i / ROPE_SIDES);
	}
	vector<uint> indices;
	for (int i = 1; i < ROPE_SIDES - 1; i++) {
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
		indices.push_back(i);
	}
	for (uint i = 0; i < ROPE_VERTICES - 1; i++) {
		for (uint j = 0; j < ROPE_SIDES; j++) {
			indices.push_back((i + 1) * ROPE_SIDES + j);
			indices.push_back(i * ROPE_SIDES + j);
		}
		indices.push_back((i + 1) * ROPE_SIDES);
		indices.push_back(i * ROPE_SIDES);
		indices.push_back((i + 1) * ROPE_SIDES);
		indices.push_back((i + 1) * ROPE_SIDES);
	}
	for (int i = 1; i < ROPE_SIDES - 1; i++) {
		indices.push_back(ROPE_VERTICES * ROPE_SIDES - ROPE_SIDES);
		indices.push_back(ROPE_VERTICES * ROPE_SIDES - ROPE_SIDES + i);
		indices.push_back(ROPE_VERTICES * ROPE_SIDES - ROPE_SIDES + i + 1);
		indices.push_back(ROPE_VERTICES * ROPE_SIDES - ROPE_SIDES + i + 1);
	}


	rope_buffer_id = mesh->add_vertex_buffer(GL_FLOAT, 3, ROPE_VERTICES * ROPE_SIDES, rope_verts.data(), GL_DYNAMIC_DRAW);
	mesh->add_vertex_buffer(GL_FLOAT, 1, ROPE_VERTICES * ROPE_SIDES, rope_verts_number.data(), GL_STATIC_DRAW);
	mesh->add_index_buffer(indices.size(), indices.data(), GL_STATIC_DRAW);
	prototype->add_mesh(mesh);
	bezier = make_shared<CatmullRom>();
	bezier->set_steps(10);
	prev_ends.reserve(5);
}

void Rope::add_rope(btRigidBody *player_rb, vec3 anchor) {

	Physic::m_soft_body_world_info.m_sparsesdf.RemoveReferences(0);
	struct Functors {
		static btSoftBody *CtorRope(const btVector3 &anchor_pos, const btVector3 &player_pos) {
			btSoftBody *psb = btSoftBodyHelpers::CreateRope(Physic::m_soft_body_world_info, anchor_pos, player_pos, ROPE_SEGMENTS, 1);
			psb->setTotalMass(500.0f);
			Physic::m_dynamics_world->addSoftBody(psb);
			return (psb);
		}
	};

	psb0 = Functors::CtorRope(to_bt(anchor), player_rb->getWorldTransform().getOrigin());
	psb0->appendAnchor(psb0->m_nodes.size() - 1, player_rb, btVector3(0, -1, 0), true);
	prev_ends.assign(5, to_glm(player_rb->getWorldTransform().getOrigin()));
	cout << "one rope" << endl;
}

void Rope::remove_rope() {
	if (psb0 != NULL)
		Physic::m_dynamics_world->removeSoftBody(psb0);
}


void Rope::draw(const shared_ptr<Shader> &shader) {
	for (int sb = 0; sb < Physic::m_dynamics_world->getSoftBodyArray().size(); sb++) {
		bezier->clear();
		btSoftBody *psb = (btSoftBody *) Physic::m_dynamics_world->getSoftBodyArray()[sb];
		btSoftBody::tNodeArray &nodes(psb->m_nodes);

		bezier->add_way_point(to_Vector(to_glm(nodes[0].m_x)));
		for (int j = 0; j < nodes.size(); j++) {
//			rope_verts[j] = to_glm(nodes[j].m_x);
//			cout << "nodes element: " << glm::to_string(rope_verts[j]) << endl;
			bezier->add_way_point(to_Vector(to_glm(nodes[j].m_x)));
		}
		bezier->add_way_point(to_Vector(to_glm(nodes[nodes.size() - 1].m_x)));
//		cout << bezier->node_count() << endl;


		rope_verts.clear();
		vec3 p0 = to_glm(bezier->node(0));
		vec3 p1 = to_glm(bezier->node(1));
//		p0 = to_glm(nodes[0].m_x);
//		p1 = to_glm(nodes[1].m_x);
		vec3 p2, dir1, dir2, cross_vec;
		vec3 dir = normalize(p1 - p0);
		if (abs(dot(dir, vec3(0, 1, 0))) < 0.7) {
			cross_vec = vec3(0, 1, 0);
		} else {
			cross_vec = vec3(1, 0, 0);
		}
		vec3 v1 = normalize(cross(dir, cross_vec));
		vec3 v2 = normalize(cross(dir, v1));
		for (uint i = 0; i < ROPE_SIDES; i++) {
			rope_verts.push_back(p0 + RADIUS * cos(2 * M_PI * i / ROPE_SIDES) * v1 + RADIUS * sin(2 * M_PI * i / ROPE_SIDES) * v2);
		}
		for (int j = 1; j < bezier->node_count() - 1; j++) {
			p0 = to_glm(bezier->node(j - 1));
			p1 = to_glm(bezier->node(j));
			p2 = to_glm(bezier->node(j + 1));
//			p0 = to_glm(nodes[j - 1].m_x);
//			p1 = to_glm(nodes[j].m_x);
//			p2 = to_glm(nodes[j + 1].m_x);
			if (p1 == p2) p2 = to_glm(bezier->node(j + 2));
			if (p0 == p1) p0 = to_glm(bezier->node(j - 2));
			dir1 = normalize(p1 - p0);
			dir2 = normalize(p2 - p1);
			dir = (dir1 + dir2) / 2;
			if (abs(dot(dir, vec3(0, 1, 0))) < 0.7) {
				cross_vec = vec3(0, 1, 0);
			} else {
				cross_vec = vec3(1, 0, 0);
			}
			v1 = normalize(cross(dir, cross_vec));
			v2 = normalize(cross(dir, v1));
			for (uint i = 0; i < ROPE_SIDES; i++) {
				rope_verts.push_back(p1 + RADIUS * cos(2 * M_PI * i / ROPE_SIDES) * v1 + RADIUS * sin(2 * M_PI * i / ROPE_SIDES) * v2);
			}
		}
		p0 = to_glm(bezier->node(bezier->node_count() - 2));
		p1 = to_glm(bezier->node(bezier->node_count() - 1));
//		p0 = to_glm(nodes[nodes.size() - 2].m_x);
//		p1 = to_glm(nodes[nodes.size() - 1].m_x);
		dir = normalize(p1 - p0);
		if (abs(dot(dir, vec3(0, 1, 0))) < 0.7) {
			cross_vec = vec3(0, 1, 0);
		} else {
			cross_vec = vec3(1, 0, 0);
		}
		v1 = normalize(cross(dir, cross_vec));
		v2 = normalize(cross(dir, v1));
		for (uint i = 0; i < ROPE_SIDES; i++) {
			rope_verts.push_back(p1 + RADIUS * cos(2 * M_PI * i / ROPE_SIDES) * v1 + RADIUS * sin(2 * M_PI * i / ROPE_SIDES) * v2);
		}
//		cout << rope_verts.size() << " " << ROPE_VERTICES * ROPE_SIDES << endl;

		if (shader) {
			prototype->use_shader(shader);
		}
		prototype->meshes[0]->update_vertex_buffer(rope_buffer_id, rope_verts.data());
		prototype->bind();
		prototype->shader->uniform("player_pos", Camera::current()->pos);
		prototype->shader->uniform("time", global_time);
		prototype->draw(mat4(1));
		prototype->unbind();
	}
}