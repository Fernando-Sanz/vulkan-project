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

	std::vector<uint32_t> getIndices() { return indices; }
	VkBuffer getVertexBuffer() { return vertexBuffer; }
	VkBuffer getIndexBuffer() { return indexBuffer; }
	TextureManager& getTextures() { return textures; }
	glm::mat4 getModelMatrix() { return model; }

	void setTextures(TextureManager& textures) { this->textures = std::move(textures); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Load a model, create its vertex and index buffer and its textures
	void create(Device device, CommandManager commandManager, std::string modelPath, TextureManager textures);

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

	Transform transform;
	glm::mat4 model;

	TextureManager textures;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Load a model from file
	void loadModel(std::string modelPath);

	// Create a buffer in GPU memory and fill it with the received data
	void createBuffer(CommandManager commandManager, VkDeviceSize bufferSize, void* bufferData,
		VkBufferUsageFlagBits bufferUsage, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
};
