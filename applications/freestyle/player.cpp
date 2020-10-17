#include "player.h"
#include "physic.h"
#include "rendering.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <cppgl/cppgl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

using namespace std;
using namespace glm;

Player::Player() {
	/* auto shader = make_shader("testcube", "shader/testcube.vs", "shader/testcube.fs"); */
	/* auto shader_callback = [shader](const std::shared_ptr<Material> &) { return shader; }; */
	/* prototype = MeshLoader::load("render-data/world/test_cube.obj", false, shader_callback); */
	trafo = glm::mat4(1);
	/* trafo = scale(trafo, vec3(0.1f)); */
	pos = glm::vec3(trafo[3]);
	dir = glm::vec3(0);

	/* for (auto &elem : prototype) { */
	/*     for (auto &m : elem->meshes) { */
	/*         positions = m->positions; */
	/*         indices = m->indices; */
	/*     } */
	/* } */
	add_rigid_body();
}

Player::~Player() {}

void Player::add_rigid_body() {

	btCollisionShape *shape = new btSphereShape(btScalar(1.));
	Physic::collision_shapes.push_back(shape);


	btTransform transform;
	transform.setIdentity();

	btScalar mass(50.0f);

	btVector3 localInertia(0, 0, 0);

	shape->calculateLocalInertia(mass, localInertia);

	transform.setOrigin(btVector3(-10, 10, 0));

	btDefaultMotionState *motion_state = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, shape, localInertia);
	player_rigid = new btRigidBody(rbInfo);

	Physic::m_dynamics_world->addRigidBody(player_rigid);

}

void Player::update() {
	trafo[3][0] = pos[0];
	trafo[3][1] = pos[1];
	trafo[3][2] = pos[2];

}

void Player::draw() {
	/* cout << "cube in draw: " << trafo[3][0] << " " << trafo[3][1] << " " << trafo[3][2] << endl; */
	for (auto &elem : prototype) {
		elem->bind();
		setup_light(elem->shader);
		elem->draw(trafo);
		elem->unbind();

	}
}
