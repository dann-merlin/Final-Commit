#include "player3d.h"
#include <map>
#include "rendering.h"
#include <glm/ext/matrix_projection.hpp>
#include <imgui/imgui.h>
#include <iostream>
#include "rope.h"
#include "jukebox.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <cppgl/cppgl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>

using namespace std;
using namespace glm;

extern bool debug_gui;
extern shared_ptr<Rope> the_rope;
extern vec3 current_anchor;
extern bool interactive_overlay;

inline vec3 to_glm(btVector3 &v) {
	return vec3(v.x(), v.y(), v.z());
}

inline btVector3 to_bt(vec3 v) {
	return btVector3(v.x, v.y, v.z);
}

namespace ImGui {
	// Plot value over time
	// Pass FLT_MAX value to draw without adding a new value
	static void PlotVar(const char *label, float value, float scale_min = FLT_MAX, float scale_max = FLT_MAX, size_t buffer_size = 120);

	// Call this periodically to discard old/unused data
	static void PlotVarFlushOldEntries();
}


class RayResultCallback : public btCollisionWorld::ClosestRayResultCallback
	{
	public:
		RayResultCallback(btCollisionObject* self)
		: btCollisionWorld::ClosestRayResultCallback(btVector3(0,0,0), btVector3(0,0,0))
		, mSelf(self)
		{
		}

		btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
		{
			if (rayResult.m_collisionObject == mSelf) return 1.0f;
			if (rayResult.m_collisionObject->getInternalType() == btCollisionObject::CO_GHOST_OBJECT) return 1.0f;
			return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
		}

	private:
		btCollisionObject* mSelf;
};


Player3D::Player3D(btDiscreteDynamicsWorld *collision_world) {
	// basic prototype and other parameters for drawing
	auto shader = make_shader("player3d", "shader/testcube.vs", "shader/testcube.fs");
	auto shader_callback = [shader](const std::shared_ptr<Material> &) { return shader; };
	prototype = MeshLoader::load("render-data/world/test_cube.obj", false, shader_callback);
	trafo = glm::mat4(1);
	trafo = scale(trafo, vec3(0.1f));
	pos = glm::vec3(trafo[3]);
	dir = glm::vec3(0);
	t.start();



	// what is this for? i dont know
	collision_world->getDispatchInfo().m_allowedCcdPenetration = 0.0001;

	// -------- Create a ghost object --------
	// set start position
	btTransform start_transform;
	start_transform.setIdentity();
	start_transform.setOrigin(btVector3(-10, 10, -10));
	// init ghost opject at position with a CapsuleShape collision
	ghost_body = new btPairCachingGhostObject();
	ghost_body->setWorldTransform(start_transform);
	characterHeight = 0.6;
	characterWidth = 1.75;
	btConvexShape *collision_shape = new btCapsuleShape(characterWidth, characterHeight);
	ghost_body->setCollisionShape(collision_shape);
	ghost_body->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);


	// -------- Create the character controller --------
	btScalar stepHeight = btScalar(0.35);
	controller = new btKinematicCharacterController(ghost_body, collision_shape, stepHeight);
	controller->setUseGhostSweepTest(false);
	controller->setGravity(btVector3(0, -9.81f * 3, 0));
	controller->warp(btVector3(-10, 10, 0));
	controller->setLinearDamping(btScalar(0));
	controller->setAngularDamping(btScalar(0));
	controller->setStepHeight(btScalar(0.1));
	controller->setMaxJumpHeight(btScalar(5));
	controller->setMaxSlope(btScalar(0.5));
	controller->setJumpSpeed(btScalar(10));
		controller->setFallSpeed(btScalar(55));
		controller->setMaxPenetrationDepth(btScalar(-0.5));

		// add object and controller to world
		collision_world->addCollisionObject(ghost_body, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter);
		collision_world->addAction(controller);


		// create rigid body shape
		rigid_body_shape = new btCapsuleShape(characterWidth, characterHeight);
		Physic::collision_shapes.push_back(rigid_body_shape);

		add_rigid_body();
	}

	void Player3D::default_input_handler(double dt_ms) {
		go_forward = false;
		go_back = false;
		go_left = false;
		go_right = false;
		// shoot_rope = false;
		jump = false;
		jetpack = false;
		draw_rigid_player = true;
		if (!interactive_overlay && not ImGui::GetIO().WantCaptureKeyboard && Camera::current()->name == "playercam") {
			// keyboard
			if (Context::key_pressed(GLFW_KEY_W))
				go_forward = true;
			if (Context::key_pressed(GLFW_KEY_S))
				go_back = true;
			if (Context::key_pressed(GLFW_KEY_A))
				go_left = true;
			if (Context::key_pressed(GLFW_KEY_D))
				go_right = true;
			static bool released_mouse = true;
			if ((Context::mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) && released_mouse && t.look() / 1000 > 1) {
				// implement rope
				shoot_rope = true;
				active_rope = true;
				released_mouse = false;
				// draw_rigid_player = true;
			}
			if(!Context::mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) { released_mouse = true; shoot_rope = false; }
			if (Context::mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT) && active_rope) {
				// switch_to_kinnematic_player = true;
				// draw_rigid_player = false;
				active_rope = false;
			}
			if (Context::key_pressed(GLFW_KEY_SPACE)) {
				jump = true;
			}
			if (Context::key_pressed(GLFW_KEY_LEFT_SHIFT)) {
				if(!on_ground) Jukebox::play_jetpack(); else Jukebox::pause_jetpack();
				jetpack = true;
			} else { Jukebox::pause_jetpack();}
			if (Context::key_pressed(GLFW_KEY_R)) {
				pull = true;
			} else {
				pull = false;
			}
			if (Context::key_pressed(GLFW_KEY_1)) {
				first_person = true;
				third_person = false;
			}
			if (Context::key_pressed(GLFW_KEY_3)) {
				third_person = true;
				first_person = false;
			}

			// if (!go_forward && !go_back && !go_left && !go_right && !jump && on_ground) {
			// 	capsule_rb->setLinearVelocity(btVector3(0,0,0));
			// }
		}
	}

	void Player3D::update(btDiscreteDynamicsWorld *collision_world) {
		// cout << "timer: " << t.look() << endl;
		auto cam = Camera::find("playercam");

		float dt = Context::frame_time() * 0.001f;
		test_on_ground(collision_world);
		/* Character stuff &*/
		if (controller) {

		}

	//	debugDrawContacts();


		if (collision_world) {
			// int maxSimSubSteps = 2; // maybe set this to 1 if idle

			///set walkDirection for our character
			btTransform xform;
			xform = ghost_body->getWorldTransform();

			btVector3 forward_dir = xform.getBasis()[2];
			forward_dir = to_bt(normalize(vec3(cam->dir.x, 0, cam->dir.z)));
			//	printf("forwardDir=%f,%f,%f\n",forwardDir[0],forwardDir[1],forwardDir[2]);
			btVector3 up_dir = xform.getBasis()[1];
			up_dir = btVector3(0, 1, 0);
			btVector3 strafe_dir = xform.getBasis()[0];
			btVector3 right_dir = to_bt(normalize(cross(cam->dir, cam->up)));
			strafe_dir = right_dir;
			forward_dir.normalize();
			right_dir.normalize();
			up_dir.normalize();
			strafe_dir.normalize();

			btVector3 walkDirection = btVector3(0.0, 0.0, 0.0);
			btScalar walkVelocity = btScalar(1.1) * 4.0; // 4 km/h -> 1.1 m/s
			btScalar walkSpeed = walkVelocity * dt;

			//rotate view
			if (go_left) {
				walkDirection -= strafe_dir;
			}

			if (go_right) {
				walkDirection += strafe_dir;
			}

			if (go_forward) {
				walkDirection += forward_dir;
				// walkDirection.setY(1.0f);
			}
			if (go_back)
				walkDirection -= forward_dir;

			if (walkDirection.length2() > 0)
				walkDirection.normalize();
			vec3 wdir = to_glm(walkDirection);
			if (wdir != vec3(0) && debug_gui)
				cout << "player dir: " << glm::to_string(wdir) << endl;

			if (on_ground) {
				btVector3 linVel = capsule_rb->getLinearVelocity();
				linVel.setX(walkDirection.x() * walkSpeed * 3 * 100.0f);
				// linVel.setY(walkDirection.y() * walkSpeed * 3 *100.0f);
				linVel.setZ(walkDirection.z() * walkSpeed * 3 * 100.0f);
				capsule_rb->setLinearVelocity(linVel);
				// btVector3 blub = capsule_rb->getLinearVelocity();
				// cout << "linear velocity: " << to_string(to_glm(blub)) << endl;
				// capsule_rb->applyCentralImpulse(walkDirection * walkSpeed * 3 * 100.0f);
				// capsule_rb->setLinearVelocity(walkDirection * walkSpeed * 3);
				if(!on_ground_last_frame) {
					Jukebox::play_sound(Jukebox::ground_hit);
					cout << "hit the ground" << endl;
				}
			} else {
				// controller->setWalkDirection(walkDirection * walkSpeed * 3);
				capsule_rb->applyCentralForce(walkDirection * walkSpeed * 3 * 10000.0f);
			}
			on_ground_last_frame = on_ground;
			capsule_rb->setGravity(btVector3(0,-9.81f,0));
			if (jump && on_ground) {
				capsule_rb->applyCentralForce(btVector3(0,1,0) * 35000.0f);
			}
			if (jetpack) {
				if (active_rope) { // if (controller->canJump()) {
					auto look_dir = Camera::find("playercam")->dir;
					auto dir_to_anchor = normalize(current_anchor - Camera::find("playercam")->pos);
					auto force_dir = -cross(dir_to_anchor, normalize(cross(dir_to_anchor, look_dir)));
					capsule_rb->applyCentralForce(to_bt(force_dir).safeNormalize() * 8000.0f);
					// controller->jump(btVector3(0, 10, 0));
				} else {
					btVector3 linVel = capsule_rb->getLinearVelocity();
					if (linVel.getY() < 0) {
						linVel.setX(linVel.getX() * 0.99f);
						linVel.setY(linVel.getY() * 0.9f);
						linVel.setZ(linVel.getZ() * 0.99f);
						capsule_rb->setLinearVelocity(linVel);
					}
					// capsule_rb->applyCentralForce(btVector3(0,1,0) * 10000.0f);

					// capsule_rb->setGravity(btVector3(0,-3,0));
				}
			}
			static float force_mulitplier = 7000.f;
			static float min_force = 300000.f;
			static float max_force = 1000000.f;
			// ImGui::Begin("rope_parameter");
			// ImGui::DragFloat("force multiplier", &force_mulitplier);
			// ImGui::DragFloat("min_force", &min_force,100000.f);
			// ImGui::DragFloat("max_force", &max_force, 1000000.f);
			// ImGui::End();
			if (pull && active_rope) {
				Jukebox::play_sound(Jukebox::sound::rope_pull);
				auto dir = (to_bt(current_anchor) - capsule_rb->getWorldTransform().getOrigin());
				float mult = std::min(force_mulitplier * dir.length() + min_force, max_force);
				capsule_rb->applyCentralForce(dir.normalize() * mult);
				the_rope->remove_rope();
				active_rope = false;
				pull = false;
			}



		// int numSimSteps = collision_world->stepSimulation(dt, maxSimSubSteps);

		//optional but useful: debug drawing
		if (collision_world)
			collision_world->debugDrawWorld();

	}

	// if (switch_to_kinnematic_player) {
	// 	controller->warp(capsule_rb->getWorldTransform().getOrigin());
	// 	// controller->setLinearVelocity(capsule_rb->getLinearVelocity());
	// 	controller->applyImpulse(capsule_rb->getLinearVelocity());
	// 	switch_to_kinnematic_player = false;
	// }


	//if (!draw_rigid_player) {
	//	pos = to_glm(ghost_body->getWorldTransform().getOrigin());
	//	trafo = glm::translate(pos);
	//	cam->pos = vec3(pos.x, pos.y + 2, pos.z) - 5 * cam->dir;
	//	// cam->pos = pos - 5 * cam->dir;
////		cout << "kinnematic pos: " << to_string(pos) << endl;
	//} else {
	if (third_person) {
		pos = to_glm(capsule_rb->getWorldTransform().getOrigin());
		trafo = glm::translate(pos);
		cam->pos = vec3(pos.x, pos.y + 2, pos.z) - 5 * cam->dir;
		// cam->pos = pos - 5 * cam->dir;
		if (debug_gui)
			cout << "player pos: " << to_string(pos) << endl;
	 } else if (first_person) {
		pos = to_glm(capsule_rb->getWorldTransform().getOrigin());
		trafo = glm::translate(pos);
		cam->pos = vec3(pos.x, pos.y + 0.5, pos.z) ;
		// cam->pos = pos - 5 * cam->dir;
		if (debug_gui)
			cout << "player pos: " << to_string(pos) << endl;
	 }

	static float pow_ = 1.f;
	float strength = clamp((float) pow(capsule_rb->getLinearVelocity().length() - 22.f, pow_) * 0.02f, 0.f, 1.f);
	Jukebox::set_volume_static_sound(Jukebox::sound::wind, strength);
	// Camera::find("playercam")->fov_degree = 70 + 30 * strength;
	velocity = strength;

	if (debug_gui) {
		// cout << to_string(pos) << endl;
		ImGui::Begin("speed");
		ImGui::DragFloat("power", &pow_, 0.01f);
		ImGui::PlotVar("Speed", capsule_rb->getLinearVelocity().length());//length(pos - old_pos));
		ImGui::PlotVarFlushOldEntries();
		ImGui::End();
	}
}


void Player3D::test_on_ground(btDiscreteDynamicsWorld *collision_world)
{
	const float TEST_DISTANCE = 3.0;

	btVector3 from = capsule_rb->getWorldTransform().getOrigin();
	btVector3 to = from - btVector3(0, TEST_DISTANCE, 0);

	// Detect ground collision and update the "on ground" status.
	RayResultCallback callback(capsule_rb);
	collision_world->rayTest(from, to, callback);

	// Check if there is something below the character.
	if (callback.hasHit())
	{
		btVector3 end = from + (to - from) * callback.m_closestHitFraction;
		// btVector3 normal = callback.m_hitNormalWorld;

		// // Slope test.
		// btScalar slopeDot = normal.dot(UP_VECTOR);
		// mOnSteepSlope = (slopeDot < mMaxClimbSlopeAngle);
		// mSlopeNormal = normal;

		// compute the distance to the floor
		float distance = btDistance(end, from);
		on_ground = (distance < 2.5);

		// // Move down.
		// if (distance < mStepHeight)
		// {
		// 	moveCharacterAlongY(-distance * 0.99999f);
		// }
		// else
		// {
		// 	moveCharacterAlongY(-mStepHeight * 0.9999f);
		// }
	}
	else
	{
		// In the air.
		on_ground = false;
		// moveCharacterAlongY(-mStepHeight);
	}
}

void Player3D::draw(const std::shared_ptr<Shader> &shader) {
	for (auto &elem : prototype) {
		elem->bind();
		//		setup_light(elem->shader);
		// draw the btKinematicCharacterController

		// elem->draw(trafo);

		// draw the rigid body
		// if (draw_rigid_player) {
			btTransform player_transform;
			glm::mat4 p;
//			capsule_rb->getMotionState()->getWorldTransform(player_transform);
			capsule_rb->getWorldTransform().getOrigin();
			player_transform.getOpenGLMatrix(glm::value_ptr(p));
			p = glm::translate(to_glm(capsule_rb->getWorldTransform().getOrigin()));
			elem->draw(p);
		// }
		elem->unbind();
	}
}


void Player3D::remove_rigid_body() {
	if (capsule_rb != NULL)
		Physic::m_dynamics_world->removeRigidBody(capsule_rb);
}

void Player3D::add_rigid_body() {
/* btCollisionShape* shape = 0; */
//	btCollisionShape *shape = new btCapsuleShape(characterWidth, characterHeight);

	btTransform capsule_transform;
	capsule_transform.setIdentity();

	btScalar mass(500.0f);

	btVector3 localInertia(0, 0, 0);

	rigid_body_shape->calculateLocalInertia(mass, localInertia);

	capsule_transform.setOrigin(ghost_body->getWorldTransform().getOrigin());

	btDefaultMotionState *cubeMotionState = new btDefaultMotionState(capsule_transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, cubeMotionState, rigid_body_shape, localInertia);
	capsule_rb = new btRigidBody(rbInfo);
	capsule_rb->setAngularFactor(btVector3(0, 0, 0));
	// btVector3 c_vel = controller->getLinearVelocity();
	// cout << "controller velocity: " << c_vel.x() << " " << c_vel.y() << " " << c_vel.z() << endl;
	// capsule_rb->applyCentralImpulse(controller->getLinearVelocity() * 100.0f);
	Physic::m_dynamics_world->addRigidBody(capsule_rb);
	cout << capsule_rb->getLinearDamping() << endl;
}


namespace ImGui {

	struct PlotVarData {
		ImGuiID ID;
		ImVector<float> Data;
		int DataInsertIdx;
		int LastFrame;

		PlotVarData() : ID(0), DataInsertIdx(0), LastFrame(-1) {}
	};

	typedef std::map<ImGuiID, PlotVarData> PlotVarsMap;
	static PlotVarsMap g_PlotVarsMap;

// Plot value over time
// Call with 'value == FLT_MAX' to draw without adding new value to the buffer
	void PlotVar(const char *label, float value, float scale_min, float scale_max, size_t buffer_size) {
		if (buffer_size == 0) {
			buffer_size = 120;
		}

		ImGui::PushID(label);
		ImGuiID id = ImGui::GetID("");

		// Lookup O(log N)
		PlotVarData &pvd = g_PlotVarsMap[id];

		// Setup
		if ((uint) pvd.Data.capacity() != buffer_size) {
			pvd.Data.resize(buffer_size);
			memset(&pvd.Data[0], 0, sizeof(float) * buffer_size);
			pvd.DataInsertIdx = 0;
			pvd.LastFrame = -1;
		}


		// Insert (avoid unnecessary modulo operator)
		if ((uint) pvd.DataInsertIdx == buffer_size)
			pvd.DataInsertIdx = 0;
		int display_idx = pvd.DataInsertIdx;
		if (value != FLT_MAX)
			pvd.Data[pvd.DataInsertIdx++] = value;

		// Draw
		int current_frame = ImGui::GetFrameCount();
		if (pvd.LastFrame != current_frame) {
			ImGui::PlotLines("##plot", &pvd.Data[0], buffer_size, pvd.DataInsertIdx, NULL, scale_min, scale_max, ImVec2(0, 40));
			ImGui::SameLine();
			ImGui::Text("%s\n%-3.4f", label, pvd.Data[display_idx]);    // Display last value in buffer
			pvd.LastFrame = current_frame;
		}

		ImGui::PopID();
	}

	void PlotVarFlushOldEntries() {
		int current_frame = ImGui::GetFrameCount();
		for (PlotVarsMap::iterator it = g_PlotVarsMap.begin(); it != g_PlotVarsMap.end();) {
			PlotVarData &pvd = it->second;
			if (pvd.LastFrame < current_frame - std::max(400, (int) pvd.Data.size()))
				it = g_PlotVarsMap.erase(it);
			else
				++it;
		}
	}
}
