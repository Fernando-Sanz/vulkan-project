#pragma once

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <unordered_map>
#include <optional>
#include <set>
//#include <cstdint> // uint32_t
#include <limits> // std::numeric_limits
#include <fstream>
#include <chrono>

#include "Window.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"
#include "imageUtils.hpp"
#include "CommandManager.hpp"
#include "StandardPipeline.hpp"
#include "FirstPassPipeline.hpp"
#include "SecondPassPipeline.hpp"
#include "UniformManager.hpp"
#include "Model.hpp"
#include "TextureManager.hpp"
#include "AppTime.hpp"
#include "eventManagement.hpp"
#include "Camera.hpp"
#include "Light.hpp"


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string POST_PROCESSING_QUAD_PATH = "C:/development/cpp/VulkanProject/assets/models/post_processing/post_processing_quad.obj";


const int MAX_FRAMES_IN_FLIGHT = 2;


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};


struct VulkanAppParams {
	std::string modelPath;
	std::string albedoTexturePath;
	std::string specularTexturePath;
	std::string normalTexturePath;
	std::vector<std::string> customTexturePaths;
	std::string firstRenderPassVertShaderPath;
	std::string firstRenderPassFragShaderPath;
	std::string secondRenderPassVertShaderPath;
	std::string secondRenderPassFragShaderPath;

	uint32_t fps = 144;
	uint32_t updateRate = 60;
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
	void run(VulkanAppParams params) {
		initWindow();
		initVulkan(params);
		mainLoop(params.fps, params.updateRate);
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
	Window window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;

	// TODO: check if could be in Window class
	// Surface
	VkSurfaceKHR surface;

	Device device;

	// Swap chain stuff
	SwapChain swapChain;

	// Images for framebuffers
	ImageObjects colorImage; // multisampling
	ImageObjects depthImage;
	TextureManager firstPassOutputManager;

	// Pipeline
	FirstPassPipeline firstPassPipeline;
	SecondPassPipeline secondPassPipeline;

	// Framebuffers
	VkFramebuffer firstPassFramebuffer;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	// Commands
	CommandManager commandManager;

	// Camera and lights
	Camera camera;
	Light spotlight;

	// Texture
	TextureManager textureManager;

	// Geometry
	Model model;
	Model postProcessingQuad;

	// Uniform
	UniformManager uniformManager;

	// Descriptor pool and sets
	VkDescriptorPool descriptorPool;
	VkDescriptorSet firstPassDescriptorSet;
	std::vector<VkDescriptorSet> descriptorSets; // destroyed with descriptorPool

	// Sync objects
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	// Resize handled manually
	int windowResizedEvent = -1;

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

		SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);

		window.create("Vulkan Render Engine", WIDTH, HEIGHT);
		addEventSubscriber(SDL_EVENT_WINDOW_RESIZED, [this](SDL_Event e) {
			windowResizedEvent = e.type;
			});
		addEventSubscriber(SDL_EVENT_WINDOW_MINIMIZED, [this](SDL_Event e) {
			windowResizedEvent = e.type;
			});

		// Hide the cursor and constrait it to the window (for easier input handling)
		SDL_SetWindowRelativeMouseMode(window.get(), true);
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// VULKAN CONFIGURATION FUNCTIONS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void initVulkan(VulkanAppParams params) {

		createInstance();
		setupDebugMessenger();

		window.createSurface(instance, &surface);

		device.pickDevice();
		swapChain.create(device, window, surface);

		commandManager.createPoolAndBuffers(device, MAX_FRAMES_IN_FLIGHT);

		createColorResources();
		createDepthResources();
		createFirstPassOutputResources();
		
		createWorldObjects(params);

		uniformManager.createBuffers(device, 1);

		firstPassPipeline.create(device, swapChain.getImageFormat(), findDepthFormat(), 1, 1, textureManager,
			params.firstRenderPassVertShaderPath, params.firstRenderPassFragShaderPath);
		secondPassPipeline.create(device, swapChain.getImageFormat(), findDepthFormat(), 0, 0, firstPassOutputManager,
			params.secondRenderPassVertShaderPath, params.secondRenderPassFragShaderPath);

		createFirstPassFramebuffer();
		createSwapChainFramebuffers();

		createDescriptorPool();
		allocateFirstPassDescriptorSets();
		allocateSecondPassDescriptorSets();

		createSyncObjects();
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// INSTANCE CREATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createInstance() {

		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but no available");
		}

		//-----------------------------------------------
		// APPLICATION INFO
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan application";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Render Engine";
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

		//-----------------------------------------------
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
		uint32_t sdlExtensionCount = 0;
		const char* const* sdlExtensions;
		sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

		std::vector<const char*> extensions(sdlExtensions, sdlExtensions + sdlExtensionCount);

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
	// IMAGE RECREATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void recreateRenderImages() {

		VkExtent2D extent = { 0,0 };
		bool minimized = windowResizedEvent == SDL_EVENT_WINDOW_MINIMIZED;
		SDL_Event sdl_event;
		while (minimized || extent.width == 0 || extent.height == 0) {
			SDL_PollEvent(&sdl_event);
			extent = window.getFramebufferSize();
			// Restore only counts if the window was minimized
			minimized = minimized && (sdl_event.type != SDL_EVENT_WINDOW_RESTORED);
		}

		vkDeviceWaitIdle(device.get());

		cleanupRenderImages();

		swapChain.create(device, window, surface);
		createColorResources();
		createDepthResources();
		createFirstPassOutputResources();
		configureSecondPassDescriptorSets();
		createSwapChainFramebuffers();
		createFirstPassFramebuffer();

		camera.updateProjection(swapChain.getExtent());
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CREATE MULTISAMPLING RESOURCES
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createColorResources() {
		VkSampleCountFlagBits msaaSamples = device.getMsaaSamples();
		VkFormat colorFormat = swapChain.getImageFormat();

		createImage(device, swapChain.getExtent().width, swapChain.getExtent().height, 1, msaaSamples,
			colorFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			colorImage.image, colorImage.memory);
		colorImage.view = createImageView(device, colorImage.image, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CREATE DEPTH RESOURCES
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createDepthResources() {
		VkSampleCountFlagBits msaaSamples = device.getMsaaSamples();
		VkFormat depthFormat = findDepthFormat();

		// Let the graphics pipeline change the layout of the image (implicit)
		// from UNDEFINED to DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		createImage(device, swapChain.getExtent().width, swapChain.getExtent().height, 1, msaaSamples,
			depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			depthImage.image, depthImage.memory);
		depthImage.view = createImageView(device, depthImage.image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
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
	// CREATE FIRST PASS OUTPUT RESOURCES
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createFirstPassOutputResources() {
		VkFormat colorFormat = swapChain.getImageFormat();
		ImageObjects texture{};

		createImage(device, swapChain.getExtent().width, swapChain.getExtent().height, 1, VK_SAMPLE_COUNT_1_BIT,
			colorFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			texture.image, texture.memory);
		texture.view = createImageView(device, texture.image, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		// CREATE THE TEXTURE
		firstPassOutputManager.create(device, commandManager);
		firstPassOutputManager.addTexture(TEXTURE_TYPE_CUSTOM_BIT, texture);
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WORLD OBJECTS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createWorldObjects(VulkanAppParams params) {
		camera.init(swapChain.getExtent());

		createTextures(params);

		model.loadModel(device, commandManager, params.modelPath);
		postProcessingQuad.loadModel(device, commandManager, POST_PROCESSING_QUAD_PATH);

		// KEYBOARD EVENTS
		addEventSubscriber(SDL_EVENT_KEY_DOWN, [this](SDL_Event e) {keyboardEventCallback(e); });
		addEventSubscriber(SDL_EVENT_KEY_UP, [this](SDL_Event e) {keyboardEventCallback(e); });
		addEventSubscriber(SDL_EVENT_MOUSE_MOTION, [this](SDL_Event e) {mouseEventCallback(e); });
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// FRAMEBUFFERS CREATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createFirstPassFramebuffer() {

		// Create framebuffer
		std::array<VkImageView, 3> attachments = {
			colorImage.view,
			depthImage.view,
			firstPassOutputManager.getCustomTexture(0).view
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = firstPassPipeline.getRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChain.getExtent().width;
		framebufferInfo.height = swapChain.getExtent().height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device.get(), &framebufferInfo, nullptr, &firstPassFramebuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create first pass framebuffer");
		}
	}

	void createSwapChainFramebuffers() {
		// Resize framebuffer
		swapChainFramebuffers.resize(swapChain.getImageCount());

		// Create framebuffers
		for (size_t i = 0; i < swapChain.getImageCount(); i++) {
			std::array<VkImageView, 3> attachments = {
				colorImage.view,
				depthImage.view,
				swapChain.getImageView(i)
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = secondPassPipeline.getRenderPass();
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChain.getExtent().width;
			framebufferInfo.height = swapChain.getExtent().height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device.get(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create swap chain framebuffer");
			}
		}
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

		//--------------------------------------------------------
		// FIRST PASS
		firstPassPipeline.recordDrawing(commandBuffer, firstPassFramebuffer, swapChain.getExtent(),
			model, firstPassDescriptorSet);

		//--------------------------------------------------------
		// SECOND PASS
		secondPassPipeline.recordDrawing(commandBuffer, swapChainFramebuffers[imageIndex], swapChain.getExtent(),
			postProcessingQuad, descriptorSets[imageIndex]);

		//--------------------------------------------------------
		// FINISH COMMAND

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer");
		}
	}

	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TEXTURES
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createTextures(VulkanAppParams params) {
		textureManager.create(device, commandManager);
		if(!params.albedoTexturePath.empty())
			textureManager.createTexture(TEXTURE_TYPE_ALBEDO_BIT, params.albedoTexturePath);
		if(!params.specularTexturePath.empty())
			textureManager.createTexture(TEXTURE_TYPE_SPECULAR_BIT, params.specularTexturePath);
		if(!params.normalTexturePath.empty())
			textureManager.createTexture(TEXTURE_TYPE_NORMAL_BIT, params.normalTexturePath);

		for (auto& texturePath : params.customTexturePaths) {
			textureManager.createTexture(TEXTURE_TYPE_CUSTOM_BIT, texturePath);
		}
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DESCRIPTOR POOL AND DESCRIPTOR SETS CREATION
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO: parameterize the pool size
	void createDescriptorPool() {
		// Sizes
		std::array<VkDescriptorPoolSize, 3> poolSizes{};
		// uniform buffer (for first pass)
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 2;
		// sampler (for each pass)
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) + 1;
		// sampler (for each pass)
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) + 2;

		// Create info
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) + 1;

		if (vkCreateDescriptorPool(device.get(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool");
		}
	}

	void allocateFirstPassDescriptorSets() {
		firstPassPipeline.allocateDescriptorSets(descriptorPool, firstPassDescriptorSet);
		firstPassPipeline.updateDescriptorSets(uniformManager, textureManager, firstPassDescriptorSet);
	}


	void allocateSecondPassDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, secondPassPipeline.getDescriptorSetLayout());

		// DESCRIPTOR SETS ALLOCATION
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device.get(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate second pass descriptor sets");
		}

		// Configure the allocated sets
		configureSecondPassDescriptorSets();
	}

	void configureSecondPassDescriptorSets(){
		// DESCRIPTOR SETS CONFIGURATION
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			secondPassPipeline.updateDescriptorSets(std::nullopt, firstPassOutputManager, descriptorSets[i]);
			/**
			// TEXTURE IMAGE SAMPLER INFO
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = firstPassOutputManager.getCustomTexture(0).view;
			imageInfo.sampler = firstPassOutputManager.getSampler();

			// DESCRIPTOR WRITES
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &imageInfo;

			// UPDATE
			vkUpdateDescriptorSets(device.get(), 1, &descriptorWrite, 0, nullptr);
			//*/
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

	void mainLoop(uint32_t fps, uint32_t updateRate) {
		if (fps <= 0 || updateRate <= 0)
			throw std::runtime_error("invalid fps or update rate, they must be grater than 0");

		// TIME MANAGEMENT
		const auto MIN_TIME_BETWEEN_FRAMES = std::chrono::nanoseconds(1000000000/fps);
		const auto MIN_TIME_BETWEEN_UPDATES = std::chrono::nanoseconds(1000000000/updateRate);
		auto timeSinceLastFrame = std::chrono::nanoseconds(0);
		auto timeSinceLastUpdate = std::chrono::nanoseconds(0);
		auto lastTime = std::chrono::high_resolution_clock::now();

		while (!window.shouldClose()) {

			// UPDATE TIMES
			auto currentTime = std::chrono::high_resolution_clock::now();
			auto deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - lastTime);
			timeSinceLastFrame += deltaTime;
			timeSinceLastUpdate += deltaTime;
			lastTime = currentTime;

			// GAME LOOP
			if (timeSinceLastFrame >= MIN_TIME_BETWEEN_FRAMES) {

				// PROCESS INPUT
				pollEvents();

				// UPDATE
				while (timeSinceLastUpdate >= MIN_TIME_BETWEEN_UPDATES) {
					updateWorld();
					timeSinceLastUpdate -= MIN_TIME_BETWEEN_UPDATES;
				}

				// DRAW
				drawFrame();

				timeSinceLastFrame -= MIN_TIME_BETWEEN_FRAMES;
			}

			// CPU IDLE TIME

		}

		vkDeviceWaitIdle(device.get());
	}

	void updateWorld() {
		AppTime::updateDeltaTime();

		camera.update();
		model.update();
		spotlight.update();
	}

	void keyboardEventCallback(SDL_Event event) {
		// TODO: create an interface to handle all keyboard event reactions
		camera.keyboardReaction(event);
		spotlight.keyboardReaction(event);
	}

	void mouseEventCallback(SDL_Event event) {
		camera.mouseReaction(event);
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
		VkResult result = vkAcquireNextImageKHR(device.get(), swapChain.get(), UINT64_MAX,
			imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		// check result to resize the swap chain
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateRenderImages();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image");
		}

		// UPDATE UNIFORMS
		uniformManager.upateBuffer(0, model.getModelMatrix(), camera, spotlight);

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
		VkSwapchainKHR swapChains[] = { swapChain.get() };

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || windowResizedEvent != -1) {
			recreateRenderImages();
			windowResizedEvent = -1;
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

		// Swap chain objects
		cleanupRenderImages();

		// Textures
		firstPassOutputManager.cleanup();
		textureManager.cleanup();

		// Uniform
		uniformManager.cleanup();

		// Descriptor pool and set layout
		vkDestroyDescriptorPool(device.get(), descriptorPool, nullptr);

		// Model
		model.cleanup();
		postProcessingQuad.cleanup();

		// Sync objects
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device.get(), renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device.get(), imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device.get(), inFlightFences[i], nullptr);
		}

		// Command pool
		commandManager.cleanup();

		// Pipeline
		firstPassPipeline.cleanup();
		secondPassPipeline.cleanup();

		// Device
		device.cleanup();

		// Instance objects
		if (enableValidationLayers)
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		window.cleanup();

		SDL_Quit();
	}

	void cleanupRenderImages() {
		// Color resources
		destroyImageObjects(device, colorImage);

		// depth resources
		destroyImageObjects(device, depthImage);

		// first pass output image
		firstPassOutputManager.destroyCustomTexture(0);

		// color attachments
		vkDestroyFramebuffer(device.get(), firstPassFramebuffer, nullptr);
		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device.get(), framebuffer, nullptr);
		}

		swapChain.cleanup();
	}

};
