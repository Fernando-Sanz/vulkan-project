#pragma once

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <array>
#include <unordered_map>
#include <optional>
#include <set>
//#include <cstdint> // uint32_t
#include <limits> // std::numeric_limits
#include <fstream>
#include <chrono>

#include "context/Window.hpp"
#include "context/Device.hpp"
#include "context/CommandManager.hpp"
#include "render/target/SwapChain.hpp"
#include "render/target/FramebufferResources.hpp"
#include "render/image/imageUtils.hpp"
#include "render/pipeline/FirstPassPipeline.hpp"
#include "render/pipeline/SecondPassPipeline.hpp"
#include "render/uniform/LightUboManager.hpp"
#include "render/uniform/ModelUboManager.hpp"
#include "render/uniform/Material.hpp"
#include "scene/Model.hpp"
#include "scene/Camera.hpp"
#include "scene/Light.hpp"
#include "system/eventManagement.hpp"
#include "time/AppTime.hpp"


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
	std::vector<TexturePaths> texturePaths;
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
	// Resize handled manually
	int windowResizedEvent = -1;

	// TODO: check if could be in Window class (VkInstance is necessary to create it)
	// Surface
	VkSurfaceKHR surface;

	Device device;
	CommandManager commandManager;

	// Swap chain stuff
	SwapChain swapChain;

	// Camera and lights
	Camera camera;
	std::array<Light, 2> lights;

	// Geometry
	Model model;
	Model postProcessingQuad;

	// Descriptor pool and sets
	VkDescriptorPool descriptorPool;

	// FIRST PASS OBJECTS
	FirstPassPipeline firstPassPipeline;
	FramebufferResources firstPassFramebuffer;
	VkDescriptorSet firstPassDescriptorSet;
	ModelUboManager modelUniforms;
	LightUboManager lightUniforms;

	// SECOND PASS OBJECTS
	SecondPassPipeline secondPassPipeline;
	std::vector<FramebufferResources> secondPassFramebuffers;
	std::vector<VkDescriptorSet> secondPassDescriptorSets;

	// Sync objects
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

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

		SDL_Init(SDL_INIT_VIDEO);

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
		commandManager.createPoolAndBuffers(device, MAX_FRAMES_IN_FLIGHT);

		swapChain.create(device, window, surface);
		
		createWorldObjects(params);

		// TODO: use a vector of models and lights
		modelUniforms.createBuffers(device, 1);
		lightUniforms.createBuffers(device, 1, lights.size());

		// the pipeline needs the texture count (models already loaded)
		firstPassPipeline.create(device, swapChain.getImageFormat(), findDepthFormat(device), model, lights.size(),
			params.firstRenderPassVertShaderPath, params.firstRenderPassFragShaderPath);

		// framebuffer needs post-processing texture image view
		createFirstPassResources();

		// second pipeline needs post-processing texture count
		secondPassPipeline.create(device, swapChain.getImageFormat(), findDepthFormat(device), postProcessingQuad, 0,
			params.secondRenderPassVertShaderPath, params.secondRenderPassFragShaderPath);
		
		createSecondPassFramebuffers();

		createDescriptorPool();
		createFirstPassDescriptorSets();
		createSecondPassDescriptorSets();

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

		// Recreate swap chain
		swapChain.create(device, window, surface);
		// Recreate framebuffers
		createFirstPassResources();
		createSecondPassFramebuffers();

		// Reset descriptor in second pass (they use the destroyed texture)
		configureSecondPassDescriptorSets();

		// Update camera with aspect ratio
		camera.updateProjection(swapChain.getExtent());
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WORLD OBJECTS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createWorldObjects(VulkanAppParams params) {
		camera.init(swapChain.getExtent());

		// MODEL
		Material modelMaterial;
		modelMaterial.setContext(device, commandManager);
		modelMaterial.createTextures(params.texturePaths[0]);
		model.create(device, commandManager, params.modelPath, modelMaterial);

		// POST-PROCESSING QUAD (the texture is set later)
		Material postProcMaterial;
		postProcMaterial.setContext(device, commandManager);
		postProcessingQuad.create(device, commandManager, POST_PROCESSING_QUAD_PATH, postProcMaterial, true);

		lights[0].setColor(glm::vec3(1.0f, 0.0f, 0.0f));

		// KEYBOARD EVENTS
		addEventSubscriber(SDL_EVENT_KEY_DOWN, [this](SDL_Event e) {keyboardEventCallback(e); });
		addEventSubscriber(SDL_EVENT_KEY_UP, [this](SDL_Event e) {keyboardEventCallback(e); });
		addEventSubscriber(SDL_EVENT_MOUSE_MOTION, [this](SDL_Event e) {mouseEventCallback(e); });
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// FRAMEBUFFERS AND POST-PROCESSING RESOURCES
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void createFirstPassResources() {
		// FRAMEBUFFER
		firstPassFramebuffer.createEmpty(device, swapChain.getExtent(), firstPassPipeline.getRenderPass(),
			swapChain.getImageFormat());

		// POST PROCESSING QUAD TEXTURES
		ImageObjects firstPassOutputImage = firstPassFramebuffer.getResolveImage();
		postProcessingQuad.getMaterial().addTexture(TEXTURE_TYPE_CUSTOM_BIT, firstPassOutputImage);
	}

	void createSecondPassFramebuffers() {
		secondPassFramebuffers.resize(MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			secondPassFramebuffers[i].createFromSwapChain(device, swapChain, i, secondPassPipeline.getRenderPass());
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
		firstPassPipeline.recordDrawing(commandBuffer, firstPassFramebuffer.get(), swapChain.getExtent(),
			model, firstPassDescriptorSet);

		//--------------------------------------------------------
		// SECOND PASS
		secondPassPipeline.recordDrawing(commandBuffer, secondPassFramebuffers[imageIndex].get(), swapChain.getExtent(),
			postProcessingQuad, secondPassDescriptorSets[imageIndex]);

		//--------------------------------------------------------
		// FINISH COMMAND

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer");
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
		poolSizes[0].descriptorCount = 3;
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

	void createFirstPassDescriptorSets() {
		firstPassPipeline.allocateDescriptorSets(descriptorPool, 1, &firstPassDescriptorSet);
		firstPassPipeline.updateDescriptorSet(modelUniforms, model.getMaterial(), lightUniforms, firstPassDescriptorSet);
	}

	void createSecondPassDescriptorSets() {
		secondPassDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		secondPassPipeline.allocateDescriptorSets(descriptorPool, MAX_FRAMES_IN_FLIGHT, secondPassDescriptorSets.data());
		configureSecondPassDescriptorSets();
	}

	void configureSecondPassDescriptorSets(){
		// DESCRIPTOR SETS CONFIGURATION
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			secondPassPipeline.updateDescriptorSet(postProcessingQuad.getMaterial(), secondPassDescriptorSets[i]);
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
		lights[1].update();
	}

	void keyboardEventCallback(SDL_Event event) {
		// TODO: create an interface to handle all keyboard event reactions
		camera.keyboardReaction(event);
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
		modelUniforms.upateBuffer(0, model, camera);
		std::vector<Light> lightVec = std::vector<Light>(lights.begin(), lights.end());
		lightUniforms.upateBuffer(0, lightVec, camera);

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

		// Model
		model.cleanup();
		postProcessingQuad.cleanup();

		// Uniform
		modelUniforms.cleanup();
		lightUniforms.cleanup();

		// Descriptor pool and set layout
		vkDestroyDescriptorPool(device.get(), descriptorPool, nullptr);

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
		// first pass output image
		postProcessingQuad.getMaterial().destroyTextures(false);

		// color attachments
		firstPassFramebuffer.cleanup();
		for (auto& framebuffer : secondPassFramebuffers) {
			framebuffer.cleanup();
		}

		swapChain.cleanup();
	}

};
