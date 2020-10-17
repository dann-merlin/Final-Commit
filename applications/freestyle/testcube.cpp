#include "testcube.h"
#include "physic.h"
#include "rendering.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <cppgl/cppgl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

using namespace std;
using namespace glm;

Testcube::Testcube() {
	auto shader = make_shader("testcube", "shader/testcube.vs", "shader/testcube.fs");
	auto shader_callback = [shader](const std::shared_ptr<Material> &) { return shader; };
//	prototype = MeshLoader::load("render-data/world/test_cube.obj", false, shader_callback);
	trafo = glm::mat4(1);
	trafo = scale(trafo, vec3(0.1f));
	pos = glm::vec3(trafo[3]);
	dir = glm::vec3(0);

	for (auto &elem : prototype) {
		for (auto &m : elem->meshes) {
			positions = m->positions;
			indices = m->indices;
		}
	}
	add_rigid_body();
}

Testcube::~Testcube() {}

void Testcube::add_rigid_body() {

	/* btCollisionShape* shape = 0; */
	btCollisionShape *shape = new btSphereShape(btScalar(1.));

	/* btVector3* pos_bt = new btVector3[positions.size()]; */
	/*  for (int i = 0; i <  positions.size(); i++) { */
	/*      auto p = positions[i]; */
	/*      pos_bt[i].setValue(p.x, p.y, p.z); */
	/*  } */

	/* const int totalTriangles = indices.size() / 3; */
	/*  const int totalVerts = positions.size(); */
	/*  int vertStride = sizeof(btVector3); */
	/*  int indexStride = 3 * sizeof(int); */

	/*  int* indices_bt = new int[indices.size()]; */
	/*  for (int i = 0; i < indices.size(); i++) { */
	/*      indices_bt[i] = indices[i]; */
	/*  } */


	/* btTriangleIndexVertexArray* data = new btTriangleIndexVertexArray(totalTriangles, indices_bt, indexStride, totalVerts, (btScalar*)&pos_bt[0].x(), vertStride); */

	/* bool useQuantizedAabbCompression = false; */

	/* shape = new btBvhTriangleMeshShape(data, useQuantizedAabbCompression, false); */
	/* shape->setMargin(0.5); */
	Physic::collision_shapes.push_back(shape);


	btTransform cube_transform;
	cube_transform.setIdentity();

	btScalar mass(10000.0f);

	btVector3 localInertia(0, 0, 0);

	shape->calculateLocalInertia(mass, localInertia);

	cube_transform.setOrigin(btVector3(10, 10, 0));

	btDefaultMotionState *cubeMotionState = new btDefaultMotionState(cube_transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, cubeMotionState, shape, localInertia);
	btRigidBody *cube = new btRigidBody(rbInfo);

	Physic::m_dynamics_world->addRigidBody(cube);


	//TRACEDEMO
	Physic::m_soft_body_world_info.m_sparsesdf.RemoveReferences(0);
	struct Functors {
		static btSoftBody *CtorRope(const btVector3 &anchor, const btVector3 &player) {
			btSoftBody *psb = btSoftBodyHelpers::CreateRope(Physic::m_soft_body_world_info, anchor, player, 8, 1);
			psb->setTotalMass(10000.0f);
			Physic::m_dynamics_world->addSoftBody(psb);
			return (psb);
		}
	};
	btSoftBody *psb0 = Functors::CtorRope(btVector3(0, 10, 0), btVector3(10, 10, 0));
	psb0->appendAnchor(psb0->m_nodes.size() - 1, cube);

}

void Testcube::update() {
	trafo[3][0] = pos[0];
	trafo[3][1] = pos[1];
	trafo[3][2] = pos[2];

}

void Testcube::draw() {
//	cout << "cube in draw: " << trafo[3][0] << " " << trafo[3][1] << " " << trafo[3][2] << endl;
	for (auto &elem : prototype) {
		elem->bind();
		setup_light(elem->shader);
		elem->draw(trafo);
		elem->unbind();

	}
}
