#include "PhysicalDevice.hpp"
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <set>
#include "VulkanApplication.hpp"


void PhysicalDevice::pickDevice() {
	VulkanApplication& instance = VulkanApplication::getInstance();

	// count the available graphics cards
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance.getVulkanInstance(), &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support");
	}

	// get the available graphics cards
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance.getVulkanInstance(), &deviceCount, devices.data());

	// find suitable graphics card
	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			this->device = device;
			msaaSamples = getMaxUsableSampleCount();
			break;
		}
	}

	if (this->device == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU");
	}
}

bool PhysicalDevice::isDeviceSuitable(VkPhysicalDevice device) {
	// IMPORTANT NOTE:
	// Here could be a customized selection process that considers
	// different aspects of the available GPUs (texture max size, 
	// separate GPU, etc)

	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	// swapchain suitability given the surface that already exists
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	// general features support
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	// the device has all queue families needed,
	// supports all extensions required,
	// has a suitable swapchain or not,
	// supports anisotropic filtering
	return indices.isComplete() && extensionsSupported && swapChainAdequate &&
		supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices PhysicalDevice::findQueueFamilies(VkPhysicalDevice device) {
	// Logic to find queue family indices
	VulkanApplication& instance = VulkanApplication::getInstance();
	QueueFamilyIndices indices;

	// query queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// check the queue families
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		// GRAPHICS QUEUE
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		// PRESENT QUEUE
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, instance.getSurface(), &presentSupport);
		if (presentSupport) {
			indices.presentFamily = i;
		}


		if (indices.isComplete()) break;

		i++;
	}

	return indices;
}

bool PhysicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

// check for CAPABILITIES, FORMATS and PRESENT MODES
SwapChainSupportDetails PhysicalDevice::querySwapChainSupport(VkPhysicalDevice device) {
	VulkanApplication& vulkanApp = VulkanApplication::getInstance();
	SwapChainSupportDetails details;

	// SURFACE CAPABILITIES
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkanApp.getSurface(), &details.capabilities);

	// SURFACE FORMATS
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanApp.getSurface(), &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanApp.getSurface(), &formatCount, details.formats.data());
	}

	// SURFACE PRESENT MODES
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanApp.getSurface(), &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanApp.getSurface(), &presentModeCount, details.presentModes.data());
	}

	return details;
}

uint32_t PhysicalDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	// Device memory properties
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type");
}

VkPhysicalDeviceProperties PhysicalDevice::getPhysicalDeviceProperties() {
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);
	return properties;
}

VkSampleCountFlagBits PhysicalDevice::getMaxUsableSampleCount() {
	VkPhysicalDeviceProperties physicalDeviceProperties = getPhysicalDeviceProperties();

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}

// Find supported format depending on desired tiling and features
VkFormat PhysicalDevice::findSupportedFormat(
	const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {

	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(device, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format");
}
