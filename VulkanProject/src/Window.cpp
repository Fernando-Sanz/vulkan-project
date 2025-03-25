#include "Window.hpp"

#include <vector>
#include <stdexcept>

#include "VulkanApplication.hpp"


void Window::framebufferResizeCallback(GLFWwindow* involvedWindow, int width, int height) {
	auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(involvedWindow));
	for (auto& update : window->framebufferResizeSubscribers)
		update(width, height);
}

void Window::createSurface(VkInstance vulkanInstance, VkSurfaceKHR* surface) {
	if (glfwCreateWindowSurface(vulkanInstance, window, nullptr, surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface");
	}
}

VkExtent2D Window::getFramebufferSize() {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	return VkExtent2D{
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};
}

int Window::shouldClose() {
	return glfwWindowShouldClose(window);
}

void Window::cleanup() {
	glfwDestroyWindow(window);
}
