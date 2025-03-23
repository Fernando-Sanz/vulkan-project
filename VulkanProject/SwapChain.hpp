#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Device.hpp"


extern struct SwapChainObjects {
	VkSwapchainKHR swapChain;
	VkFormat imageFormat;
	VkExtent2D extent;
	size_t imageCount;
	std::vector<VkImage> images; // destroyed with swap chain
	std::vector<VkImageView> imageViews;
}swapChainObjects;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// METHODS

void createSwapChainObjects(SwapChainObjects& swapChainObjects, Device device, GLFWwindow* window, VkSurfaceKHR surface);

void cleanupSwapChain(SwapChainObjects& swapChainObjects, Device device);
