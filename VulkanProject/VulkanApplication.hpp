#pragma once

//#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <unordered_map>
#include <optional>
#include <set>
//#include <cstdint> // uint32_t
#include <limits> // std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include <fstream>

#include <stb_image.h>
#include <tiny_obj_loader.h>

#include "Device.hpp"

#include "Vertex.hpp"
#include "UniformManager.hpp"
#include "CommandManager.hpp"
#include "GraphicsPipeline.hpp"


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string VERT_SHADER_PATH = "shaders/vert.spv";
const std::string FRAG_SHADER_PATH = "shaders/frag.spv";

const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";

const int MAX_FRAMES_IN_FLIGHT = 2;


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

}



class VulkanApplication {

public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

	static VulkanApplication& getInstance() {
		static VulkanApplication instance;
		return instance;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	VkInstance getVulkanInstance() { return instance; }
	VkSurfaceKHR getSurface() { return surface; }

private:

	// Instance objects
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;

	// Surface
	VkSurfaceKHR surface;

	Device device;

	// Swapchain objects
	VkSwapchainKHR swapChain;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImage> swapChainImages; // destroyed with swap chain
	std::vector<VkImageView> swapChainImageViews;

	// Render target (multisampling)
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	// Depth resources
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	// Pipeline
	GraphicsPipeline graphicsPipeline;

	// Framebuffers
	std::vector<VkFramebuffer> swapChainFramebuffers;

	// Commands
	CommandManager commandManager;

	// Texture objects
	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	// Geometry objects
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	// Uniform
	UniformManager uniformManager;

	// Descriptor pool and sets
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets; // destroyed with descriptorPool

	// Sync objects
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	// Resize handled manually
	bool framebufferResized = false;

	// Frame tracking
	uint32_t currentFrame = 0;



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CONSTRUCTOR AND OPERATORS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanApplication() {}

	// Delete copy and assignment (for singleton)
	VulkanApplication(const VulkanApplication&) = delete;
	VulkanApplication& operator=(const VulkanApplication&) = delete;


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WINDOW CONFIGURATION FUNCTIONS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// VULKAN CONFIGURATION FUNCTIONS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void initVulkan() {
		createInstance();
		setupDebugMessenger();

		createSurface();

		device.pickDevice();

		createSwapChain();
		createImageViews();

		graphicsPipeline.create(device, swapChainImageFormat, findDepthFormat(), VERT_SHADER_PATH, FRAG_SHADER_PATH);

		commandManager.createPoolAndBuffers(device, MAX_FRAMES_IN_FLIGHT);

		createColorResources();
		createDepthResources();
		createFramebuffers();

		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		
		loadModel();
		createVertexBuffer();
		createIndexBuffer();

		uniformManager.createBuffers(device, MAX_FRAMES_IN_FLIGHT);
		createDescriptorPool();
		allocateDescriptorSets();

		createSyncObjects();
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// INSTANCE CREATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createInstance() {

		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but no available");
		}

		// APPLICATION INFO
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan application";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "My left egg's engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// GET EXTENSIONS
		auto extensions = getRequiredExtensions();

		// SET THE VALIDATION LAYERS
		uint32_t layerCount = 0;
		const char* const* layers = nullptr;
		if (enableValidationLayers) {
			layerCount = static_cast<uint32_t>(validationLayers.size());
			layers = validationLayers.data();
		}
		else {
			layerCount = 0;
		}

		// INSTANCE DEBUG MESSENGER
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		VkDebugUtilsMessengerCreateInfoEXT* pNext = nullptr;
		if (enableValidationLayers) {
			populateDebugMessengerCreateInfo(debugCreateInfo);
			pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}

		//-----------------------------------------------
		// CREATE INFO
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		createInfo.enabledLayerCount = layerCount;
		createInfo.ppEnabledLayerNames = layers;

		createInfo.pNext = pNext;

		// CREATE VULKAN INSTANCE
		if (vkCreateInstance(&createInfo, nullptr, &instance)) {
			throw std::runtime_error("failed to create instance.");
		}
	}

	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		// add debug utils extension (if debugging)
		if (enableValidationLayers)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		// show required extensions
		std::cout << "Required extensions:" << std::endl;
		for (const auto& extension : extensions) {
			std::cout << extension << std::endl;
		}

		return extensions;
	}

	bool checkValidationLayerSupport() {
		// GET AVAILABLE LAYERS
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// COMPARE THE AVAILABLE LAYERS WITH THE REQUESTED ONES
		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DEBUG MESSENGER CREATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void setupDebugMessenger() {
		if (!enableValidationLayers) return;

		// create info
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger");
		}
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		//createInfo.messageSeverity =
		//	VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		//	VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		bool filterMessages = true;

		// it only shows error and warning messages
		if (filterMessages && messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			std::cerr << "VALIDATION LAYER: " << pCallbackData->pMessage << std::endl;
			//std::cerr << "\tREFERED TO OBJECT: " << pCallbackData->pObjects->pObjectName << std::endl;
		}

		return VK_FALSE;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SURFACE CREATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface");
		}
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SWAP CHAIN CREATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = device.querySwapChainSupport();

		// MAJOR PROPERTIES
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

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
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device.get(), swapChain, &imageCount, swapChainImages.data());

		// STORE THE FORMAT AND EXTENT
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

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

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
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

	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {

		// CREATE INFO
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;

		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;

		// color components mapping
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		// CREATE IMAGE VIEW
		VkImageView imageView;
		if (vkCreateImageView(device.get(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	void recreateSwapChain() {
		int width = 0, height = 0;
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device.get());

		cleanupSwapChainObjects();

		createSwapChain();
		createImageViews();
		createColorResources();
		createDepthResources();
		createFramebuffers();
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// FRAMEBUFFERS CREATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createFramebuffers() {
		// Resize framebuffer
		swapChainFramebuffers.resize(swapChainImageViews.size());

		// Create framebuffers
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			std::array<VkImageView, 3> attachments = {
				colorImageView,
				depthImageView,
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = graphicsPipeline.getRenderPass();
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device.get(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer");
			}
		}
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CREATE MULTISAMPLING RESOURCES
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createColorResources() {
		VkSampleCountFlagBits msaaSamples = device.getMsaaSamples();
		VkFormat colorFormat = swapChainImageFormat;

		createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples,
			colorFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			colorImage, colorImageMemory);
		colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CREATE DEPTH RESOURCES
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createDepthResources() {
		VkSampleCountFlagBits msaaSamples = device.getMsaaSamples();
		VkFormat depthFormat = findDepthFormat();

		// Let the graphics pipeline change the layout of the image (implicit)
		// from UNDEFINED to DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples,
			depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			depthImage, depthImageMemory);
		depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
	}

	VkFormat findDepthFormat() {
		return device.findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	bool hasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// RECORD DRAWING
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {

		// BEGIN RECORDING
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer");
		}

		// DRAWING

		// render pass info
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = graphicsPipeline.getRenderPass();
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		std::array<VkClearValue, 2> clearValues = {};
		// The order must be identical to the attachments one
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// drawing commands
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.get());

		// vertex buffers
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		// index buffer
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// viewport and scissor stage
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChainExtent.width);
		viewport.height = static_cast<float>(swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			graphicsPipeline.getLayout(), 0, 1, &descriptorSets[currentFrame], 0, nullptr);

		// draw geometry
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		// end recording
		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer");
		}
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TEXTURE IMAGE CREATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createTextureImage() {
		// LOAD IMAGE
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4/*bytes per channel*/;
		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image");
		}

		std::cout << "Loaded texture ";
		std::cout << texWidth << "x" << texHeight << std::endl;

		// COPY IMAGE TO STAGING BUFFER
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device.get(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device.get(), stagingBufferMemory);

		// cleanup old pixel array
		stbi_image_free(pixels);

		// CREATE IMAGE
		createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT,
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			textureImage, textureImageMemory);

		// COPY THE IMAGE FROM STAGING BUFFER
		transitionImageLayout(textureImage,
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			mipLevels);
		copyBufferToImage(stagingBuffer, textureImage,
			static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		// transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

		// CLEAN UP STAGING STUFF
		vkDestroyBuffer(device.get(), stagingBuffer, nullptr);
		vkFreeMemory(device.get(), stagingBufferMemory, nullptr);

		// GENERATE MIPMAPS
		generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
	}

	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSample,
		VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
		VkImage& image, VkDeviceMemory& imageMemory) {

		// CREATE IMAGE
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = numSample;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// TODO: check alternatives for posible format problems (e.g. SRGB not supported)
		if (vkCreateImage(device.get(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image");
		}

		// ALLOCATE IMAGE MEMORY
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.get(), image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device.get(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory");
		}

		// BIND MEMORY WITH IMAGE
		vkBindImageMemory(device.get(), image, imageMemory, 0);
	}

	void transitionImageLayout(VkImage image, VkFormat format,
		VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {

		VkCommandBuffer commandBuffer = commandManager.beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		commandManager.endSingleTimeCommands(commandBuffer);
	}

	// TODO: record all copy operations in the same command buffer to allow
	// asynchronously execution (layout transitions too)
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		VkCommandBuffer commandBuffer = commandManager.beginSingleTimeCommands();

		// DEFINE REGION
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		// COPY THE IMAGE
		vkCmdCopyBufferToImage(commandBuffer,
			buffer, image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		commandManager.endSingleTimeCommands(commandBuffer);
	}

	void createTextureImageView() {
		textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
	}

	// MIPMAPS
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {

		// CHECK FILTERING SUPPORTED BY FORMAT
		VkFormatProperties formatProperties;
		device.getPhysicalDeviceFormatProperties(imageFormat, &formatProperties);
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("ERROR: texture image format does not support linear blitting");
		}

		VkCommandBuffer commandBuffer = commandManager.beginSingleTimeCommands();

		// Reusable image memory barrier
		VkImageMemoryBarrier barrier{};
		// static fields
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++) {

			// BARRIER BETWEEN OPERATIONS
			// variables fields
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			// BLIT OPERATION
			VkImageBlit blit{};
			// src
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			// dst
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			// BARRIER BETWEEN OPERATIONS
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		// BARRIER FOR LAST MIP LEVEL
		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		// SUBMIT COMMANDS
		commandManager.endSingleTimeCommands(commandBuffer);
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TEXTURE SAMPLER CREATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createTextureSampler() {

		// CREATE INFO
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		// anisotropic filtering
		VkPhysicalDeviceProperties properties = device.getPhysicalDeviceProperties();
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		// for shadow mapping and similar
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f; // optional
		samplerInfo.minLod = 0.0f; // optional
		samplerInfo.maxLod = static_cast<float>(mipLevels);

		// CREATE SAMPLER
		if (vkCreateSampler(device.get(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("ERROR: failed to create texture sampler");
		}
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// VERTEX BUFFER CREATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void loadModel() {
		// Elements loaded from obj file
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		// LOAD THE MODEL
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
			throw std::runtime_error(warn + err);
		}


		// POPULLATE THE VERTICES

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				// position
				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				// texture coordinates
				//	the vertical component is flipped for correct visualization (OBJ to Vulkan conversion)
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				// base color
				vertex.color = { 1.0f, 1.0f, 1.0f };

				// check if the vertex already exists and store the index
				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}

		std::cout << "Model loaded: " << vertices.size() << " vertices" << std::endl;
	}

	// TODO: allocate more than one resource from a single call
	// TODO: store all the data in a single buffer and use offsets in calls with them
	void createVertexBuffer() {


		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		//--------------------------------------------
		// STAGING BUFFER
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		// copy data to the buffer
		void* data;
		vkMapMemory(device.get(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(device.get(), stagingBufferMemory);

		//--------------------------------------------
		// VERTEX BUFFER
		device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer, vertexBufferMemory);

		commandManager.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		// Clean up
		vkDestroyBuffer(device.get(), stagingBuffer, nullptr);
		vkFreeMemory(device.get(), stagingBufferMemory, nullptr);
	}

	void createIndexBuffer() {

		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		//--------------------------------------------
		// STAGING BUFFER
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		// copy data to the buffer
		void* data;
		vkMapMemory(device.get(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(device.get(), stagingBufferMemory);

		//--------------------------------------------
		// VERTEX BUFFER
		device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer, indexBufferMemory);

		commandManager.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		// Clean up
		vkDestroyBuffer(device.get(), stagingBuffer, nullptr);
		vkFreeMemory(device.get(), stagingBufferMemory, nullptr);
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DESCRIPTOR POOL AND DESCRIPTOR SETS CREATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createDescriptorPool() {
		// Sizes
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		// uniform buffer
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		// image sampler
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		// Create info
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(device.get(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool");
		}
	}

	void allocateDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, graphicsPipeline.getDescriptorSetLayout());

		// DESCRIPTOR SETS ALLOCATION
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device.get(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets");
		}

		// DESCRIPTOR SETS CONFIGURATION
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			// UNIFORM BUFFER INFO
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformManager.getBuffer(i);
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			// TEXTURE IMAGE SAMPLER INFO
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textureImageView;
			imageInfo.sampler = textureSampler;

			// DESCRIPTOR WRITES
			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			// buffer
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pImageInfo = nullptr; // Not used
			descriptorWrites[0].pTexelBufferView = nullptr; // Not used

			// image sampler
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			// UPDATE
			vkUpdateDescriptorSets(device.get(), static_cast<uint32_t>(descriptorWrites.size()),
				descriptorWrites.data(), 0, nullptr);
		}
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SYNC OBJECTS CREATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Starts signaled

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(device.get(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device.get(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device.get(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create sync objects");
			}
		}
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// MAIN LOOP
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void mainLoop() {

		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(device.get());
	}

	void drawFrame() {
		/* DRAW FRAME STEPS
		* Wait for the previous frame to finish
		* Acquire an image from the swap chain
		* Record a command buffer which draws the scene onto that image
		* Submit the recorded command buffer
		* Present the swap chain image
		*/

		//---------------------------------------
		// WAIT FOR THE PREVIOUS FRAME TO FINISH
		vkWaitForFences(device.get(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		//---------------------------------------
		// ACQUIRE AN IMAGE FROM THE SWAP CHAIN
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device.get(), swapChain, UINT64_MAX,
			imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		// check result to resize the swap chain
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image");
		}

		// TODO: change this way and use push constants
		uniformManager.upateBuffer(currentFrame, swapChainExtent.width, swapChainExtent.height);

		vkResetFences(device.get(), 1, &inFlightFences[currentFrame]);

		//---------------------------------------
		// RECORD A COMMAND BUFFER
		VkCommandBuffer commandBuffer = commandManager.getBuffer(currentFrame);
		vkResetCommandBuffer(commandBuffer, 0);
		recordCommandBuffer(commandBuffer, imageIndex);

		//---------------------------------------
		// SUBMIT THE COMMAND BUFFER
		VkSemaphore waitSemaphores[] = { /*id0*/imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { /*id0*/VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer");
		}

		//---------------------------------------
		// PRESENT THE IMAGE
		VkSwapchainKHR swapChains[] = { swapChain };

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image");
		}

		//---------------------------------------
		// UPDATE CURRENT FRAME
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLEANUP FUNCTIONS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void cleanup() {

		// Color resources (multisampling)
		vkDestroyImageView(device.get(), colorImageView, nullptr);
		vkDestroyImage(device.get(), colorImage, nullptr);
		vkFreeMemory(device.get(), colorImageMemory, nullptr);

		// Swap chain objects
		cleanupSwapChainObjects();

		// Texture
		vkDestroySampler(device.get(), textureSampler, nullptr);
		vkDestroyImageView(device.get(), textureImageView, nullptr);
		vkDestroyImage(device.get(), textureImage, nullptr);
		vkFreeMemory(device.get(), textureImageMemory, nullptr);

		// Uniform
		uniformManager.cleanup();

		// Descriptor pool and set layout
		vkDestroyDescriptorPool(device.get(), descriptorPool, nullptr);

		// Geometry buffers
		vkDestroyBuffer(device.get(), vertexBuffer, nullptr);
		vkFreeMemory(device.get(), vertexBufferMemory, nullptr);
		vkDestroyBuffer(device.get(), indexBuffer, nullptr);
		vkFreeMemory(device.get(), indexBufferMemory, nullptr);

		// Sync objects
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device.get(), renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device.get(), imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device.get(), inFlightFences[i], nullptr);
		}

		// Command pool
		commandManager.cleanup();

		// Pipeline
		graphicsPipeline.cleapup();

		// Device
		device.cleanup();

		// Instance objects
		if (enableValidationLayers)
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}

	void cleanupSwapChainObjects() {
		// depth resources
		vkDestroyImageView(device.get(), depthImageView, nullptr);
		vkDestroyImage(device.get(), depthImage, nullptr);
		vkFreeMemory(device.get(), depthImageMemory, nullptr);

		// color attachments
		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device.get(), framebuffer, nullptr);
		}

		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device.get(), imageView, nullptr);
		}

		vkDestroySwapchainKHR(device.get(), swapChain, nullptr);
	}

};
