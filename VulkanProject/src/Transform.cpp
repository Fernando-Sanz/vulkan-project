#include "Transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


const glm::vec3 Transform::RIGHT = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 Transform::FORWARD = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 Transform::UP = glm::vec3(0.0f, 0.0f, 1.0f);

void Transform::changeOrientation(glm::mat3 transformation) {
	lookAt = transformation * lookAt;
	up = transformation * up;
	right = glm::cross(lookAt, up);
}

void Transform::changeOrientation(glm::vec3 newLookAt) {
	glm::quat q = glm::rotation(lookAt, newLookAt);
	glm::mat3 rotationMatrix = glm::toMat3(q);

	changeOrientation(rotationMatrix);
}
