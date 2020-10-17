#ifndef CPPGL_BONETRANSFORM_H
#define CPPGL_BONETRANSFORM_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

class BoneTransform {
public:
	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;

	BoneTransform();

	BoneTransform(glm::vec3 pos, glm::quat rot, glm::vec3 scale);

	glm::mat4 get_local_transform();

	static std::shared_ptr<BoneTransform> interpolate(BoneTransform frameA, BoneTransform frameB, float a);
};


#endif //CPPGL_BONETRANSFORM_H
