#pragma once

#include "Window.hpp"
#include "Device.hpp"
#include "CommandManager.hpp"


class VulkanInstance {
public:

	VkInstance get() { return instance; }

	void create(const char* applicationName);

	void cleanup();

private:

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;

	void createInstance(const char* applicationName);
	void setupDebugMessenger();
};
