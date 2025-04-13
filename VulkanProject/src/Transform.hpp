#pragma once

#include <glm/glm.hpp>


struct Transform {
	glm::vec3 position;
	glm::vec3 lookAt;
	glm::vec3 up;
	glm::vec3 right;

	static const glm::vec3 X;
	static const glm::vec3 Y;
	static const glm::vec3 Z;

	Transform() :
		position(glm::vec3(0.0f)),
		lookAt(glm::vec3(0.0f, 1.0f, 0.0f)),
		up(glm::vec3(0.0f, 0.0f, 1.0f)),
		right(glm::cross(lookAt, up))
	{}

	void changeOrientation(glm::mat3 transformation);
	void changeOrientation(glm::vec3 newLookAt);
};
