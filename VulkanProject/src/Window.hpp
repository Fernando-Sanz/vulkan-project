#pragma once

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>

#include <functional>


class Window {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	SDL_Window* get() { return window; }



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS
	
	void create(char* title, int width, int height);

	template <typename F>
	void subscribeFramebufferResizedEvent(F updateFunction) {
		framebufferResizeSubscribers.push_back(updateFunction);
	}

	void createSurface(VkInstance vulkanInstance, VkSurfaceKHR* surface);

	// Get the resolution in pixels
	VkExtent2D getFramebufferSize();

	int shouldClose();

	void cleanup();

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	SDL_Window* window;
	std::vector<std::function<void(int, int)>> framebufferResizeSubscribers;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	friend bool framebufferResizeCallback(void* userdata, SDL_Event* event);

};
