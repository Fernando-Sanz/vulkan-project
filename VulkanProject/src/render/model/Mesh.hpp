#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string>

#include "context/Device.hpp"
#include "context/CommandManager.hpp"
#include "render/model/Vertex.hpp"


class Mesh {
public:
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	std::vector<uint32_t> getIndices() { return indices; }
	VkBuffer getIndexBuffer() const { return indexBuffer; }
	VkBuffer getVertexBuffer() const { return vertexBuffer; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Load a model, create its vertex and index buffer and its textures. If useRawVertexData is true then the transform
	// is not initialized (the shader will use the raw vertex data without transformations)
	void create(Device device, CommandManager commandManager, std::string modelPath);

	// Destroy Vulkan and other objects
	void cleanup();

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	// TODO: move to MeshManager, store here an ID instead
	Device device;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Load a model from file
	void loadModel(std::string modelPath);

	// Create a buffer in GPU memory and fill it with the received data
	void createBuffer(CommandManager commandManager, VkDeviceSize bufferSize, void* bufferData,
		VkBufferUsageFlagBits bufferUsage, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
};
