#pragma once

#include <glm/glm.hpp>
#include <SDL3/SDL.h>

#include "Transform.hpp"
#include "SwapChain.hpp"


class Light {
public:

	glm::vec3 getPosition() { return transform.position; }
	glm::vec3 getColor() { return color; }
	glm::vec3 getDirection() { return transform.lookAt; }

	void setColor(glm::vec3 color) { this->color = color; }

	Light();

	void update();
	void keyboardReaction(SDL_Event event);

private:

	Transform transform;
	glm::vec3 color;

};
