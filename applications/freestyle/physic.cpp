#include "physic.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"
#include "btBulletDynamicsCommon.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "rendering.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <cppgl/cppgl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

using namespace std;
using namespace glm;
btDiscreteDynamicsWorld* Physic::dynamics_world;
btAlignedObjectArray<btCollisionShape*> Physic::collision_shapes;
btSoftBodyWorldInfo Physic::m_soft_body_world_info;
btSoftRigidDynamicsWorld* Physic::m_dynamics_world;
const int maxProxies = 32766;

Physic::Physic() {
	// create necessary components for a bullet physics world
    btDefaultCollisionConfiguration* collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    m_soft_body_world_info.m_dispatcher = dispatcher;

    btVector3 worldAabbMin(-1000,-1000,-1000);
    btVector3 worldAabbMax(1000,1000,1000);

    btBroadphaseInterface* m_broadphase = new btAxisSweep3(worldAabbMin, worldAabbMax, maxProxies);
    m_soft_body_world_info.m_broadphase = m_broadphase;

    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
    
    btSoftBodySolver* soft_body_solver = 0;

    // create the bullet world
    Physic::dynamics_world = new btSoftRigidDynamicsWorld(dispatcher, m_broadphase, solver, collisionConfiguration, soft_body_solver);
    m_dynamics_world = (btSoftRigidDynamicsWorld*) dynamics_world;

    m_dynamics_world->getDispatchInfo().m_enableSPU = true;
    m_dynamics_world->setGravity(btVector3(0,-9.81f,0));
    m_soft_body_world_info.m_gravity.setValue(0,-9.81f,0);

    m_soft_body_world_info.m_sparsesdf.Initialize();

}
    
    
