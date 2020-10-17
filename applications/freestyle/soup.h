#ifndef CPPGL_SOUP_H
#define CPPGL_SOUP_H

#include <cppgl/drawelement.h>
#include <cppgl/framebuffer.h>

class Soup {
public:
	float height = -200;
	static int w, h;
	float rising_speed = 1;
	glm::vec3 pos = glm::vec3(0);
	glm::mat4 trafo = glm::mat4(1);
	static std::shared_ptr<Drawelement> prototype;
	static std::shared_ptr<Framebuffer> fb;
	static std::shared_ptr<Shader> update_soup_shader;

	Soup(float height);

	static void init_soup();

	void update_soup_texture();

	void update();

	void draw();
};


#endif //CPPGL_SOUP_H
