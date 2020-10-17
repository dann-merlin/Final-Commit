#pragma once
#include <cppgl/cppgl.h>
#include "physic.h"

class Player {
  public:
    Player();
    ~Player();
    void draw();
    void update();
    void add_rigid_body();

    std::vector<std::shared_ptr<Drawelement>> prototype;
    //data
    btRigidBody* player_rigid;
    std::vector<glm::vec3> positions;
    std::vector<uint32_t> indices;
    glm::vec3 dir;
    glm::vec3 pos;
    glm::mat4 trafo;

};
