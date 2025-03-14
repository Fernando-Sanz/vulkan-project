#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>

#include "Device.hpp"


class CommandManager {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	// Get command buffer at specified position
	VkCommandBuffer getCommandBuffer(int index) { return commandBuffers[index]; }


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Create command pool and allocate command buffers
	void createCommandPoolAndBuffers(Device device, int bufferCount);

	// Begin a command buffer
	VkCommandBuffer beginSingleTimeCommands();

	// End a command buffer
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	// Destroy Vulkan and other objects
	void cleanup();

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	Device device;

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers; // destroyed with command pool


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	void allocateCommandBuffers(int bufferCount);
};
