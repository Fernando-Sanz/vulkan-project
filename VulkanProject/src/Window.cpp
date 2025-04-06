#include "Window.hpp"

#include <SDL3/SDL_vulkan.h>

#include <vector>
#include <stdexcept>


bool framebufferResizeCallback(void* userdata, SDL_Event* event) {
	if (event->type != SDL_EVENT_WINDOW_RESIZED) return true;

	auto window = reinterpret_cast<Window*>(userdata);

	int width, height;
	SDL_GetWindowSize(window->window, &width, &height);

	for (auto& update : window->framebufferResizeSubscribers)
		update(width, height);

	return true;
}

void Window::create(char* title, int width, int height) {
	window = SDL_CreateWindow(title,
		width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	// Add subscribers
	SDL_AddEventWatch(framebufferResizeCallback, this);
}

void Window::createSurface(VkInstance vulkanInstance, VkSurfaceKHR* surface) {
	if (!SDL_Vulkan_CreateSurface(window, vulkanInstance, nullptr, surface)) {
		throw std::runtime_error("failed to create window surface");
	}
}

VkExtent2D Window::getFramebufferSize() {
	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	return VkExtent2D{
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};
}

int Window::shouldClose() {
	SDL_Event event;
	SDL_PollEvent(&event);
	return event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED;
}

void Window::cleanup() {
	SDL_DestroyWindow(window);
}
