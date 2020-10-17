#ifndef CPPGL_WORLD3D_H
#define CPPGL_WORLD3D_H

#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "btBulletDynamicsCommon.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"
#include <cppgl/cppgl.h>

class World3D {
public:
	World3D();
	//~Physic();

	//data
	static btDiscreteDynamicsWorld* dynamics_world;
	static btSoftBodyWorldInfo m_soft_body_world_info;
	static btSoftRigidDynamicsWorld* m_dynamics_world;
	static btAlignedObjectArray<btCollisionShape*> collision_shapes;


};


#endif //CPPGL_WORLD3D_H
