#pragma once

#include <glm/glm.hpp>


// TODO: review the fields (rotation)
struct Transform {
	Transform* parent = nullptr;

	glm::vec3 position = Transform::ORIGIN;
	glm::vec3 lookAt = Transform::Y;
	glm::vec3 up = Transform::Z;
	glm::vec3 right = Transform::X;
	glm::vec3 scale = glm::vec3(1.0f);

	static const glm::vec3 ORIGIN;
	static const glm::vec3 X;
	static const glm::vec3 Y;
	static const glm::vec3 Z;

	Transform() :
		position(Transform::ORIGIN),
		lookAt(Transform::Y),
		right(Transform::X),
		up(Transform::Z),
		scale(glm::vec3(1.0f))
	{}

	void changeOrientation(glm::mat3 transformation);
	void changeOrientation(glm::vec3 newLookAt);
};

glm::mat4 createWorldMatrix(const Transform* transform);

glm::mat4 createModelMatrix(const Transform* transform);
