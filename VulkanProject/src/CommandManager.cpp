#include "CommandManager.hpp"


void CommandManager::createPoolAndBuffers(Device device, int bufferCount) {
	this->device = device;

	//-----------------------------------------
	// Create graphics command pool
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = device.findQueueFamilies().graphicsFamily.value();

	if (vkCreateCommandPool(device.get(), &poolInfo, nullptr, &pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool");
	}

	//-----------------------------------------
	// Allocate command buffers
	allocateCommandBuffers(bufferCount);
}

VkCommandBuffer CommandManager::beginSingleTimeCommands() {
	//-----------------------------------------
	// Allocate buffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device.get(), &allocInfo, &commandBuffer);

	//-----------------------------------------
	// Begin recording
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void CommandManager::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	//-----------------------------------------
	// End recording
	vkEndCommandBuffer(commandBuffer);

	//-----------------------------------------
	// Submit and wait execution
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device.getGraphicsQueue());

	//-----------------------------------------
	// Free command buffer
	vkFreeCommandBuffers(device.get(), pool, 1, &commandBuffer);
}

void CommandManager::allocateCommandBuffers(int bufferCount) {
	buffers.resize(bufferCount);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)buffers.size();

	if (vkAllocateCommandBuffers(device.get(), &allocInfo, buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers");
	}
}

void CommandManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

	// Begin command
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	// Copy command
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	// Submit
	endSingleTimeCommands(commandBuffer);
}

void CommandManager::cleanup() {
	vkDestroyCommandPool(device.get(), pool, nullptr);
}
