#include "scene/Camera.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "time/AppTime.hpp"


Camera::Camera() {
	view = glm::mat4(1.0f);
	projection = glm::mat4(1.0f);
}

void Camera::init(VkExtent2D extent) {
	computeView();
	updateProjection(extent);
}

void Camera::update() {
	transform->position += movingDirection * speed * AppTime::deltaTime();
	computeView();
}

void Camera::keyboardReaction(SDL_Event event) {
	float xFactor = glm::dot(transform->lookAt, Transform::X);
	float yFactor = glm::dot(transform->lookAt, Transform::Y);
	glm::vec3 forwardDirection = xFactor * Transform::X + yFactor * Transform::Y;
	forwardDirection = glm::normalize(forwardDirection);

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
		}
	}
	movingDirection = glm::vec3(0.0f);
	movingDirection += forwardDirection * (float)moving.forward;
	movingDirection -= forwardDirection * (float)moving.backward;
	movingDirection += transform->right * (float)moving.toRight;
	movingDirection -= transform->right * (float)moving.toLeft;
	if (movingDirection != glm::vec3(0.0f))
		movingDirection = glm::normalize(movingDirection);
}

// TODO: cap the angle (do not allow the 180 degrees rotation)
void Camera::mouseReaction(SDL_Event event) {
	SDL_MouseMotionEvent mouse = event.motion;
	
	glm::mat4 rot = glm::mat4(1.0f);
	if (mouse.yrel != 0) {
		float factor = 0.0f;
		factor = (mouse.yrel < 0) ? 1.0f : -1.0f;
		float verticalAngle = angleSpeed * AppTime::deltaTime() * sensitivity * factor;
		rot = glm::rotate(rot, verticalAngle, transform->right);
	}
	if (mouse.xrel != 0) {
		float factor = 0.0f;
		factor = (mouse.xrel < 0) ? 1.0f : -1.0f;
		float horizontalAngle = angleSpeed * AppTime::deltaTime() * sensitivity * factor;
		rot = glm::rotate(glm::mat4(1.0f), horizontalAngle, Transform::Z) * rot;
	}
	transform->changeOrientation(glm::mat3(rot));
}

void Camera::computeView() {
	if (transform == nullptr) return;
	
	view = glm::lookAt(
		transform->position, transform->position + transform->lookAt, transform->up);
}

void Camera::computeProjection() {
	if (transform == nullptr) return;

	float aspectRatio = extent.width / (float)extent.height;
	projection = glm::perspective(
		glm::radians(45.0f), aspectRatio, 0.1f, 10.0f);
	projection[1][1] *= -1; // non-OpenGL GLM usage adjustment
}
