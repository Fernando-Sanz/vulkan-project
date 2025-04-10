#pragma once

#include <glm/glm.hpp>

#include "Transform.hpp"
#include "SwapChain.hpp"


class Light {
public:

	glm::vec3 getPosition() { return transform.position; }
	glm::vec3 getColor() { return color; }
	glm::vec3 getDirection() { return transform.lookAt; }

	Light();

	void update();

private:

	Transform transform;
	glm::vec3 color;

};
