#pragma once

#include <glm/glm.hpp>


struct Transform {
	glm::vec3 position;
	glm::vec3 lookAt;
	glm::vec3 up;
	glm::vec3 right;
};
