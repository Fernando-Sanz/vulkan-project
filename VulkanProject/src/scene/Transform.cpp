#include "Transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


const glm::vec3 Transform::ORIGIN = glm::vec3(0.0f);
const glm::vec3 Transform::X = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 Transform::Y = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 Transform::Z = glm::vec3(0.0f, 0.0f, 1.0f);

void Transform::changeOrientation(glm::mat3 transformation) {
	lookAt = glm::normalize(transformation * lookAt);
	up = glm::normalize(transformation * up);
	right = glm::normalize(glm::cross(lookAt, up));
}

void Transform::changeOrientation(glm::vec3 newLookAt) {
	glm::quat q = glm::rotation(lookAt, newLookAt);
	glm::mat3 rotationMatrix = glm::toMat3(q);

	changeOrientation(rotationMatrix);
}

glm::mat4 createModelMatrix(const Transform* transform) {
	glm::mat4 model = glm::mat4(1.0f);

	// rotation
	// the up vector is in the Z axis
	model[0] = glm::vec4(transform->right, 0.0f);
	model[1] = glm::vec4(transform->lookAt, 0.0f);
	model[2] = glm::vec4(transform->up, 0.0f);
	// translation
	model[3] = glm::vec4(transform->position, 1.0f);
	// scale
	model = model * glm::scale(glm::mat4(1.0f), transform->scale);

	return model;
}

glm::mat4 createWorldMatrix(const Transform* transform) {

	const Transform* current = transform;
	glm::mat4 worldMatrix = glm::mat4(1.0f);
	while (current) {
		worldMatrix = createModelMatrix(current) * worldMatrix;
		current = current->parent;
	}

	return worldMatrix;
}
