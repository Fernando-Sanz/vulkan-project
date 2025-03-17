#include "Device.hpp"
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <set>
#include "VulkanApplication.hpp"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PHYSICAL DEVICE METHODS

void Device::pickDevice() {
	VulkanApplication& instance = VulkanApplication::getInstance();

	//-----------------------------------------
	// Get available graphics cards
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance.getVulkanInstance(), &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance.getVulkanInstance(), &deviceCount, devices.data());

	//-----------------------------------------
	// Find suitable graphics card
	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			msaaSamples = getMaxUsableSampleCount();
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU");
	}

	//-----------------------------------------
	// Create logical device
	createLogicalDevice();
}

bool Device::isDeviceSuitable(VkPhysicalDevice device) {
	// IMPORTANT NOTE:
	// Here could be a customized selection process that considers different
	// aspects of the available GPUs (texture max size, separate GPU, etc)

	// QUEUE FAMILIES
	QueueFamilyIndices indices = findQueueFamilies(device);

	// EXTENSIONS SUPPORT
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	// SWAP CHAIN SUPPORT
	// Swapchain suitability given the surface that already exists
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	// GENERAL FEATURES SUPPORT
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() &&				// all queue families needed
		extensionsSupported &&					// all extensions required
		swapChainAdequate &&					// swapchain support
		supportedFeatures.samplerAnisotropy;	// supports anisotropy
}

QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device) {
	// Logic to find queue family indices
	VulkanApplication& instance = VulkanApplication::getInstance();
	QueueFamilyIndices indices;

	//-----------------------------------------
	// Get queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	//-----------------------------------------
	// Check queue families needed
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

		// CHECK TO EXIT
		if (indices.isComplete()) break;

		i++;
	}

	return indices;
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	//-----------------------------------------
	// Get device extensions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	//-----------------------------------------
	// Check required extensions
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device) {
	VulkanApplication& vulkanApp = VulkanApplication::getInstance();

	//-----------------------------------------
	// SURFACE CAPABILITIES
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkanApp.getSurface(), &details.capabilities);

	//-----------------------------------------
	// SURFACE FORMATS
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanApp.getSurface(), &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanApp.getSurface(), &formatCount, details.formats.data());
	}

	//-----------------------------------------
	// SURFACE PRESENT MODES
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanApp.getSurface(), &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanApp.getSurface(), &presentModeCount, details.presentModes.data());
	}

	return details;
}

uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {

	// Device memory properties
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	// Check the types and properties
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type");
}

VkPhysicalDeviceProperties Device::getPhysicalDeviceProperties() {
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	return properties;
}

VkSampleCountFlagBits Device::getMaxUsableSampleCount() {
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

VkFormat Device::findSupportedFormat(
	const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {

	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format");
}

void Device::getPhysicalDeviceFormatProperties(VkFormat imageFormat, VkFormatProperties* formatProperties) {
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, formatProperties);
}

void Device::cleanup() {
	vkDestroyDevice(logicalDevice, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LOGICAL DEVICE METHODS

void Device::createLogicalDevice() {

	//-----------------------------------------
	// QUEUE CREATE INFOS

	// unique queue families
	QueueFamilyIndices indices = findQueueFamilies();
	std::set<uint32_t> queueFamilies = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value() };

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : queueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	//-----------------------------------------
	// SPECIFY DEVICE FEATURES
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	//-----------------------------------------
	// DEVICE CREATE INFO
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// IMPORTANT NOTE:
	// In previous vulkan implementations, the logical device create info
	// used to have layer indicators for validation layers (device specific).
	// Now that isn't longer the case, but there are people who recommend to
	// indicate it for compatibility with older implementations.
	// Here it is ignores but it is something to have in count.

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device");
	}

	//-----------------------------------------
	// GET THE QUEUES
	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
}

void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {

	//-----------------------------------------
	// CREATE BUFFER
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer");
	}

	//-----------------------------------------
	// ALLOCATE BUFFER MEMORY
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory");
	}

	//-----------------------------------------
	// ASSOCIATE BUFFER WITH MEMORY
	vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
}
