#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>


class Window {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	GLFWwindow* get() { return window; }



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS
	
	template <typename F>
	void create(void* userPointer, int width, int height) {
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

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

	GLFWwindow* window;
	std::vector<std::function<void(int, int)>> framebufferResizeSubscribers;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

};
