#include "Camera.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


Camera::Camera() {
	transform.position = glm::vec3(0.0f, 3.0, 0.5f);
	transform.lookAt = glm::vec3(0.0f, -1.0f, 0.0f);
	transform.up = glm::vec3(0.0f, 0.0f, 1.0f);
	transform.right = glm::cross(transform.lookAt, transform.up);

	view = glm::mat4();
	projection = glm::mat4();
}

void Camera::init(SwapChain swapChain) {
	this->swapChain = swapChain;
	updateMatrices();
}

void Camera::update() {

	updateMatrices();
}

void Camera::keyboardReaction(SDL_Event event) {
	float speed = 0.25f;
	switch (event.key.key) {
	case SDLK_W:
		transform.position += transform.lookAt * speed;
		break;
	case SDLK_A:
		transform.position += -transform.right * speed;
		break;
	case SDLK_S:
		transform.position += -transform.lookAt * speed;
		break;
	case SDLK_D:
		transform.position += transform.right * speed;
		break;
	}
}

void Camera::updateMatrices() {
	computeView();
	computeProjection();
}

void Camera::computeView() {
	view = glm::lookAt(
		transform.position, transform.position + transform.lookAt, transform.up);
}

void Camera::computeProjection() {
	float aspectRatio = swapChain.getExtent().width / (float)swapChain.getExtent().height;
	projection = glm::perspective(
		glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);
	projection[1][1] *= -1; // non-OpenGL GLM usage adjustment
}
