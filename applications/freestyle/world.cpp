#include "world.h"
#include "physic.h"
#include "rendering.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <cppgl/cppgl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

using namespace std;

extern shared_ptr<Shader> world3d_dpn_shader;
extern float global_time;

World::World() {
    auto shader = make_shader("world", "shader/world.vs", "shader/world.fs");
    auto shader_callback = [shader](const std::shared_ptr<Material> &) { return world3d_dpn_shader; };
    prototype = MeshLoader::load("render-data/world/world_spread.obj", false, shader_callback);
    // prototype = MeshLoader::load("render-data/world/world_high.obj", false, shader_callback);
    //prototype = MeshLoader::load("render-data/world/world_flat.obj", false);
    trafo = glm::mat4(1);
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

World::~World() {}

void World::add_rigid_body() {
    
    btCollisionShape* shape = 0;

   btVector3* pos_bt = new btVector3[positions.size()];
    for (uint i = 0; i <  positions.size(); i++) {
        auto p = positions[i]; 
        pos_bt[i].setValue(p.x, p.y, p.z);
    }

   const int totalTriangles = indices.size() / 3;
    const int totalVerts = positions.size();
    int vertStride = sizeof(btVector3);
    int indexStride = 3 * sizeof(int);
   
    int* indices_bt = new int[indices.size()];
    for (uint i = 0; i < indices.size(); i++) {
        indices_bt[i] = indices[i];
    }


   btTriangleIndexVertexArray* data = new btTriangleIndexVertexArray(totalTriangles, indices_bt, indexStride, totalVerts, (btScalar*)&pos_bt[0].x(), vertStride);

   bool useQuantizedAabbCompression = false;

   shape = new btBvhTriangleMeshShape(data, useQuantizedAabbCompression);
   /* shape->setMargin(0.5); */
   Physic::collision_shapes.push_back(shape);


   btTransform transform;
   transform.setIdentity();
   transform.setOrigin(btVector3(0,0,0));
   
   btScalar mass(0.0f);
    
   btVector3 localInertia(0,0,0);

   btDefaultMotionState* cubeMotionState = new btDefaultMotionState(transform);
   btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, cubeMotionState, shape, localInertia);
   btRigidBody* world = new btRigidBody(rbInfo);
    
   Physic::m_dynamics_world->addRigidBody(world);

}

void World::update() {
    trafo[3][0] = pos[0];
    trafo[3][1] = pos[1];
    trafo[3][2] = pos[2];
}

void World::draw(shared_ptr<Shader> &shader) {
    for (auto &elem : prototype) {
		elem->use_shader(shader);
	  	elem->bind();
		elem->shader->uniform("time", global_time);
	  	setup_light(elem->shader);
	  	elem->draw(trafo);
	  	elem->unbind();
	}
}
