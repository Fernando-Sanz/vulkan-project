#pragma once

#include "Device.hpp"
#include "CommandManager.hpp"
#include "Vertex.hpp"


class Model {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	std::vector<uint32_t> getIndices() { return indices; }
	VkBuffer getVertexBuffer() { return vertexBuffer; }
	VkBuffer getIndexBuffer() { return indexBuffer; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Load a model and create vertex and index buffer
	void loadModel(Device device, CommandManager commandManager, std::string modelPath);

	// Destroy Vulkan and other objects
	void cleanup();

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	Device device;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Create a buffer in GPU memory and fill it with the received data
	void createModelBuffer(CommandManager commandManager, VkDeviceSize bufferSize, void* bufferData,
		VkBufferUsageFlagBits bufferUsage, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
};
