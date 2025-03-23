#include "SwapChain.hpp"

#include <algorithm> // for std::clamp

#include "imageUtils.hpp"


namespace {

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { // triple buffering
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR; // it is guaranteed to be available
	}

	VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
		// Vulkan sets the extent to the resolution of the window,
		// however some window managers change the value to uint32 max value
		// to indicate that the value can change (it is not interesting in this case)
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return capabilities.currentExtent;

		// get the resolution in pixels
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		// try to adjust the window resolution to the swap chain extent bounds
		actualExtent.width = std::clamp(
			actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(
			actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

	void createSwapChain(SwapChainObjects& swapChainObjects, Device device, GLFWwindow* window, VkSurfaceKHR surface) {
		SwapChainSupportDetails swapChainSupport = device.querySwapChainSupport();

		// MAJOR PROPERTIES
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(window, swapChainSupport.capabilities);

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
		if (vkCreateSwapchainKHR(device.get(), &createInfo, nullptr, &swapChainObjects.swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain");
		}

		// RETRIEVE THE IMAGES
		vkGetSwapchainImagesKHR(device.get(), swapChainObjects.swapChain, &imageCount, nullptr);
		swapChainObjects.images.resize(imageCount);
		swapChainObjects.imageCount = imageCount;
		vkGetSwapchainImagesKHR(device.get(), swapChainObjects.swapChain, &imageCount, swapChainObjects.images.data());

		// STORE THE FORMAT AND EXTENT
		swapChainObjects.imageFormat = surfaceFormat.format;
		swapChainObjects.extent = extent;
	}

	void createImageViews(SwapChainObjects& swapChainObjects, Device device) {
		swapChainObjects.imageViews.resize(swapChainObjects.imageCount);

		for (size_t i = 0; i < swapChainObjects.imageCount; i++) {
			swapChainObjects.imageViews[i] = createImageView(
				device, swapChainObjects.images[i], swapChainObjects.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}
}

void createSwapChainObjects(SwapChainObjects& swapChainObjects, Device device, GLFWwindow* window, VkSurfaceKHR surface) {
	createSwapChain(swapChainObjects, device, window, surface);
	createImageViews(swapChainObjects, device);
}

void cleanupSwapChain(SwapChainObjects& swapChainObjects, Device device) {
	for (auto imageView : swapChainObjects.imageViews) {
		vkDestroyImageView(device.get(), imageView, nullptr);
	}

	vkDestroySwapchainKHR(device.get(), swapChainObjects.swapChain, nullptr);
}
