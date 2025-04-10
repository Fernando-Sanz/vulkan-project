#pragma once

#include <glm/glm.hpp>
#include <SDL3/SDL.h>

#include "Transform.hpp"
#include "SwapChain.hpp"


class Camera{
public:
	glm::mat4 getView() { return view; }
	glm::mat4 getProjection() { return projection; }

	Camera();

	void init(SwapChain swapChain);

	void update();
	void keyboardReaction(SDL_Event event);

private:
	SwapChain swapChain;

	Transform transform;
	glm::mat4 view;
	glm::mat4 projection;

	void computeView();
	void computeProjection();
	void updateMatrices();

};
