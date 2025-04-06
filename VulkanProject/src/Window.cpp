#include "Window.hpp"

#include <SDL3/SDL_vulkan.h>

#include <vector>
#include <stdexcept>

#include "eventManagement.hpp"


void Window::create(char* title, int width, int height) {
	window = SDL_CreateWindow(title,
		width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	// Subscribe to close window event
	addEventSubscriber(SDL_EVENT_WINDOW_CLOSE_REQUESTED, [this](SDL_Event e) {closeRequested = true; });
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

void Window::cleanup() {
	SDL_DestroyWindow(window);
}
