#pragma once

#include "Device.hpp"
#include "window.hpp"


class SwapChain {

public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	VkSwapchainKHR get() { return swapChain; }

	VkFormat getImageFormat() { return imageFormat; }

	size_t getImageCount() { return images.size(); }

	VkImageView getImageView(size_t index) { return imageViews[index]; }

	VkExtent2D getExtent() { return extent; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	void create(Device device, Window window, VkSurfaceKHR surface);

	void cleanup();

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	Device device;

	VkSwapchainKHR swapChain;
	VkFormat imageFormat;
	VkExtent2D extent;
	std::vector<VkImage> images; // destroyed with swap chain
	std::vector<VkImageView> imageViews;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	void createSwapChain(Window window, VkSurfaceKHR surface);

	VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	VkExtent2D chooseExtent(Window window, const VkSurfaceCapabilitiesKHR& capabilities);

	void createImageViews();

};
