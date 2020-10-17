#pragma once
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "btBulletDynamicsCommon.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"
#include <cppgl/cppgl.h>

class Physic {
  public:
    Physic();
    //~Physic();

    void add_shape(std::vector<glm::vec3> obj_positions, std::vector<uint32_t> obj_indices);

    //data
    static btDiscreteDynamicsWorld* dynamics_world;
    static btSoftBodyWorldInfo m_soft_body_world_info;
    static btSoftRigidDynamicsWorld* m_dynamics_world;
    static btAlignedObjectArray<btCollisionShape*> collision_shapes;


};
