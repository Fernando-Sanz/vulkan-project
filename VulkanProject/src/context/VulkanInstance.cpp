#include "VulkanInstance.hpp"

#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};


namespace {

	bool checkValidationLayerSupport() {
		// GET AVAILABLE LAYERS
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		/**
		for (const auto& layer : availableLayers) {
			std::cout << "- " << layer.layerName << ": ";
			std::cout << layer.description << std::endl;
		}//*/

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

	std::vector<const char*> getRequiredExtensions() {
		// SDL gives the needed extensions
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

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
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
}

void VulkanInstance::create(const char* applicationName) {
	createInstance(applicationName);
	setupDebugMessenger();
}

void VulkanInstance::createInstance(const char* applicationName) {

	// VALIDATION LAYERS
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but no available");
	}

	//-----------------------------------------------
	// APPLICATION INFO
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = applicationName;
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

void VulkanInstance::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	// create info
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger");
	}
}

void VulkanInstance::cleanup() {
	if (enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	vkDestroyInstance(instance, nullptr);
}
