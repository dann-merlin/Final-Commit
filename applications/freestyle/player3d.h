#ifndef CPPGL_PLAYER3D_H
#define CPPGL_PLAYER3D_H

#include <cppgl/cppgl.h>
#include "physic.h"

#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

class Player3D {
public:
	//data
	std::vector<std::shared_ptr<Drawelement>> prototype;
	btCollisionShape *rigid_body_shape;
	btRigidBody *player;
	std::vector<glm::vec3> positions;
	std::vector<uint32_t> indices;
	glm::vec3 dir;
	glm::vec3 pos;
	glm::mat4 trafo;
	float velocity = 0.f;
	btKinematicCharacterController *controller;
	btPairCachingGhostObject *ghost_body;
	btScalar characterHeight;
	btScalar characterWidth;
	btRigidBody *capsule_rb;
	float default_player_movement_speed = 0.01;
	bool go_forward = false, go_left = false, go_right = false, go_back = false, jump = false, shoot_rope = false, draw_rigid_player = false, switch_to_kinnematic_player = false, first_person = true, third_person = false, on_ground = true, on_ground_last_frame = true, active_rope = false, jetpack = false, pull = false;
	Timer t = Timer();

	Player3D(btDiscreteDynamicsWorld *collision_world);

	void default_input_handler(double dt_ms);

	void update(btDiscreteDynamicsWorld *collision_world);

	void test_on_ground(btDiscreteDynamicsWorld *collision_world);
	void draw(const std::shared_ptr<Shader> &shader);

	void remove_rigid_body();
	void add_rigid_body();
};

#endif //CPPGL_PLAYER3D_H
