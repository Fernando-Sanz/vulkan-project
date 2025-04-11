#pragma once

#include <glm/glm.hpp>
#include <SDL3/SDL.h>

#include "Transform.hpp"
#include "SwapChain.hpp"


struct MovementState {
	bool forward = 0;
	bool backward = 0;
	bool toRight = 0;
	bool toLeft = 0;
};

class Camera{
public:
	glm::mat4 getView() { return view; }
	glm::mat4 getProjection() { return projection; }

	void updateProjection(VkExtent2D extent) { this->extent = extent; computeProjection(); }

	Camera();

	void init(VkExtent2D extent);

	void update();
	void keyboardReaction(SDL_Event event);
	void mouseReaction(SDL_Event event);

private:
	VkExtent2D extent;

	Transform transform;
	glm::mat4 view;
	glm::mat4 projection;

	void computeView();
	void computeProjection();

	MovementState moving;
	glm::vec3 movingDirection = glm::vec3(0.0f);
	float speed = 1.5f;
	float angleSpeed = glm::radians(90.0f);
	float sensitivity = 5.0f;
};
