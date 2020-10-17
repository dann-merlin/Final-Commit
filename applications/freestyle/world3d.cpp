#include "world3d.h"
#include "rendering.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

using namespace std;
using namespace glm;
btDiscreteDynamicsWorld *World3D::dynamics_world;
btAlignedObjectArray<btCollisionShape *> World3D::collision_shapes;
btSoftBodyWorldInfo World3D::m_soft_body_world_info;
btSoftRigidDynamicsWorld *World3D::m_dynamics_world;
const int maxProxies = 32766;

World3D::World3D() {
	btCollisionShape* groundShape = new btBoxShape(btVector3(50,3,50));
	collision_shapes.push_back(groundShape);

	auto m_collisionConfiguration = new btDefaultCollisionConfiguration();
	auto m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
	btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);
	btAxisSweep3* sweepBP = new btAxisSweep3(worldMin,worldMax);
	auto m_overlappingPairCache = sweepBP;

	auto m_constraintSolver = new btSequentialImpulseConstraintSolver();
	dynamics_world = new btDiscreteDynamicsWorld(m_dispatcher,m_overlappingPairCache,m_constraintSolver,m_collisionConfiguration);
	dynamics_world->getDispatchInfo().m_allowedCcdPenetration=0.0001f;

}