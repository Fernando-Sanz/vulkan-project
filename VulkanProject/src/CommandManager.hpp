#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>

#include "Device.hpp"


class CommandManager {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	// Get command buffer at specified position
	VkCommandBuffer getBuffer(int index) { return buffers[index]; }


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Create command pool and allocate command buffers
	void createPoolAndBuffers(Device device, int bufferCount);

	// Begin a command buffer
	VkCommandBuffer beginSingleTimeCommands();

	// End a command buffer
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	// Copy the content of a buffer to other
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	// Destroy Vulkan and other objects
	void cleanup();

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	Device device;

	VkCommandPool pool;
	std::vector<VkCommandBuffer> buffers; // destroyed with command pool


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	void allocateCommandBuffers(int bufferCount);
};
