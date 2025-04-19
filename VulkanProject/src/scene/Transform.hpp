#pragma once

#include <glm/glm.hpp>


// TODO: review the fields (rotation)
struct Transform {
	glm::vec3 position;
	glm::vec3 lookAt;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 scale;

	static const glm::vec3 X;
	static const glm::vec3 Y;
	static const glm::vec3 Z;

	Transform() :
		position(glm::vec3(0.0f)),
		lookAt(Transform::Y),
		up(Transform::Z),
		right(Transform::X),
		scale(glm::vec3(1.0f))
	{}

	void changeOrientation(glm::mat3 transformation);
	void changeOrientation(glm::vec3 newLookAt);
};

glm::mat4 createModelMatrix(Transform transform);
