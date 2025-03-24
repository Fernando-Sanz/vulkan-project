#include "SwapChain.hpp"

#include <algorithm> // for std::clamp

#include "imageUtils.hpp"


void SwapChain::create(Device device, Window window, VkSurfaceKHR surface) {
	this->device = device;

	createSwapChain(window, surface);
	createImageViews();
}

VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR SwapChain::choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { // triple buffering
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR; // it is guaranteed to be available
}

VkExtent2D SwapChain::chooseExtent(Window window, const VkSurfaceCapabilitiesKHR& capabilities) {
	// Vulkan sets the extent to the resolution of the window,
	// however some window managers change the value to uint32 max value
	// to indicate that the value can change (it is not interesting in this case)
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;

	VkExtent2D actualExtent = window.getFramebufferSize();

	// try to adjust the window resolution to the swap chain extent bounds
	actualExtent.width = std::clamp(
		actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(
		actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtent;
}

void SwapChain::createSwapChain(Window window, VkSurfaceKHR surface) {
	SwapChainSupportDetails swapChainSupport = device.querySwapChainSupport();

	// MAJOR PROPERTIES
	VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = choosePresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseExtent(window, swapChainSupport.capabilities);

	// NUMBER OF IMAGES
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	// CREATE INFO
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// how to handle images between queues:
	QueueFamilyIndices indices = device.findQueueFamilies();
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	// image transform (rotation, flip, ...)
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

	// blending with other windows
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE; // pixels obscured by a window in front

	// for resize
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// CREATE THE SWAP CHAIN
	if (vkCreateSwapchainKHR(device.get(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain");
	}

	// RETRIEVE THE IMAGES
	vkGetSwapchainImagesKHR(device.get(), swapChain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(device.get(), swapChain, &imageCount, images.data());

	// STORE THE FORMAT AND EXTENT
	imageFormat = surfaceFormat.format;
	this->extent = extent;
}

void SwapChain::createImageViews() {
	imageViews.resize(images.size());

	for (size_t i = 0; i < images.size(); i++) {
		imageViews[i] = createImageView(
			device, images[i], imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

void SwapChain::cleanup() {
	for (auto imageView : imageViews) {
		vkDestroyImageView(device.get(), imageView, nullptr);
	}

	vkDestroySwapchainKHR(device.get(), swapChain, nullptr);
}
