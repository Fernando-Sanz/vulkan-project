#pragma once

#include "context/Device.hpp"
#include "context/CommandManager.hpp"
#include "render/vertex/Vertex.hpp"
#include "Transform.hpp"
#include "render/uniform/TextureManager.hpp"


class Model {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	bool useRawVertexData() { return transform == nullptr; }
	// Return a copy of the transform. Since the transform can be nullptr, it should be checked with useRawVertexData() method
	Transform getTransform() { return *transform; }

	std::vector<uint32_t> getIndices() { return indices; }
	VkBuffer getVertexBuffer() { return vertexBuffer; }
	
	VkBuffer getIndexBuffer() { return indexBuffer; }
	TextureManager& getTextures() { return textures; }

	void setTextures(TextureManager& textures) { this->textures = std::move(textures); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Load a model, create its vertex and index buffer and its textures. If useRawVertexData is true then the transform
	// is not initialized (the shader will use the raw vertex data without transformations)
	void create(Device device, CommandManager commandManager, std::string modelPath, TextureManager textures,
		bool useRawVertexData = false);

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

	Transform* transform = nullptr;

	TextureManager textures;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Load a model from file
	void loadModel(std::string modelPath);

	// Create a buffer in GPU memory and fill it with the received data
	void createBuffer(CommandManager commandManager, VkDeviceSize bufferSize, void* bufferData,
		VkBufferUsageFlagBits bufferUsage, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
};
