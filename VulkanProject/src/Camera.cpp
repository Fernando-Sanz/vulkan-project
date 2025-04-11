#include "Camera.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "AppTime.hpp"


Camera::Camera() {
	transform.position = glm::vec3(0.0f, 3.0, 0.5f);
	transform.lookAt = glm::vec3(0.0f, -1.0f, 0.0f);
	transform.up = glm::vec3(0.0f, 0.0f, 1.0f);
	transform.right = glm::cross(transform.lookAt, transform.up);

	view = glm::mat4();
	projection = glm::mat4();
}

void Camera::init(VkExtent2D extent) {
	computeView();
	updateProjection(extent);
}

void Camera::update() {
	transform.position += movingDirection * speed * AppTime::deltaTime();
	transform.changeOrientation(rotation);
	computeView();
}

void Camera::keyboardReaction(SDL_Event event) {
	float xFactor = glm::dot(transform.lookAt, Transform::RIGHT);
	float yFactor = glm::dot(transform.lookAt, Transform::FORWARD);
	glm::vec3 forwardDirection = xFactor * Transform::RIGHT + yFactor * Transform::FORWARD;
	forwardDirection = glm::normalize(forwardDirection);

	xFactor = glm::dot(transform.right, Transform::RIGHT);
	yFactor = glm::dot(transform.right, Transform::FORWARD);
	glm::vec3 rightDirection = xFactor * Transform::RIGHT + yFactor * Transform::FORWARD;
	rightDirection = glm::normalize(rightDirection);

	if (event.type == SDL_EVENT_KEY_DOWN) {
		switch (event.key.key) {
		case SDLK_W:
			moving.forward = 1;
			break;
		case SDLK_S:
			moving.backward = 1;
			break;
		case SDLK_A:
			moving.toLeft = 1;
			break;
		case SDLK_D:
			moving.toRight = 1;
			break;
		case SDLK_UP:
			rotating.up = 1;
			break;
		case SDLK_DOWN:
			rotating.down = 1;
			break;
		case SDLK_LEFT:
			rotating.toLeft = 1;
			break;
		case SDLK_RIGHT:
			rotating.toRight = 1;
			break;
		}
	}
	else if (event.type == SDL_EVENT_KEY_UP) {
		switch (event.key.key) {
		case SDLK_W:
			moving.forward = 0;
			break;
		case SDLK_S:
			moving.backward = 0;
			break;
		case SDLK_A:
			moving.toLeft = 0;
			break;
		case SDLK_D:
			moving.toRight = 0;
			break;
		case SDLK_UP:
			rotating.up = 0;
			break;
		case SDLK_DOWN:
			rotating.down = 0;
			break;
		case SDLK_LEFT:
			rotating.toLeft = 0;
			break;
		case SDLK_RIGHT:
			rotating.toRight = 0;
			break;
		}
	}
	movingDirection = glm::vec3(0.0f);
	movingDirection += forwardDirection * (float)moving.forward;
	movingDirection -= forwardDirection * (float)moving.backward;
	movingDirection += transform.right * (float)moving.toRight;
	movingDirection -= transform.right * (float)moving.toLeft;
	if (movingDirection != glm::vec3(0.0f))
		movingDirection = glm::normalize(movingDirection);

	glm::mat4 rot = glm::mat4(1.0f);
	rot = glm::rotate(rot, glm::radians(angleSpeed * AppTime::deltaTime()) * (float)rotating.toLeft, Transform::UP);
	rot = glm::rotate(rot, glm::radians(-angleSpeed * AppTime::deltaTime()) * (float)rotating.toRight, Transform::UP);
	rot = glm::rotate(rot, glm::radians(angleSpeed * AppTime::deltaTime()) * (float)rotating.up, transform.right);
	rot = glm::rotate(rot, glm::radians(-angleSpeed * AppTime::deltaTime()) * (float)rotating.down, transform.right);
	rotation = glm::mat3(rot);
}

void Camera::computeView() {
	view = glm::lookAt(
		transform.position, transform.position + transform.lookAt, transform.up);
}

void Camera::computeProjection() {
	float aspectRatio = extent.width / (float)extent.height;
	projection = glm::perspective(
		glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);
	projection[1][1] *= -1; // non-OpenGL GLM usage adjustment
}
