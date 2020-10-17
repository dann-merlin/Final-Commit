#pragma once

#ifndef CPPGL_SKYBOX_H
#define CPPGL_SKYBOX_H

#include <cppgl/cppgl.h>
#include <cppgl/stb_image.h>

class Skybox {
public:
    //data
    static GLuint vao, sky_tex;
	static std::shared_ptr<Drawelement> prototype;
    //data end

    Skybox();
    ~Skybox();

    void draw(std::shared_ptr<Shader> & shader);
};


#endif //CPPGL_SKYBOX_H
