#pragma once
#include <cppgl/cppgl.h>

class Testcube {
  public:
    Testcube();
    ~Testcube();
    void draw();
    void update();
    void add_rigid_body();

    std::vector<std::shared_ptr<Drawelement>> prototype;
    //data
    std::vector<glm::vec3> positions;
    std::vector<uint32_t> indices;
    glm::vec3 dir;
    glm::vec3 pos;
    glm::mat4 trafo;
};
