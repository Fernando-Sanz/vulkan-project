#pragma once

#include "scene/Module.hpp"
#include "render/vertex/Vertex.hpp"
#include "Transform.hpp"
#include "render/uniform/Material.hpp"


class Model : public Module {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	bool useRawVertexData() { return transform == nullptr; }

	std::vector<uint32_t> getIndices() { return indices; }
	VkBuffer getVertexBuffer() { return vertexBuffer; }
	
	VkBuffer getIndexBuffer() { return indexBuffer; }
	Material& getMaterial() { return material; }

	void setMaterial(Material& material) { this->material = std::move(material); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Load a model, create its vertex and index buffer and its textures. If useRawVertexData is true then the transform
	// is not initialized (the shader will use the raw vertex data without transformations)
	void create(Device device, CommandManager commandManager, std::string modelPath, Material material,
		bool useRawVertexData = false);

	void setOwner(Entity* owner) override {
		Module::setOwner(owner);
		transform = nullptr;
	}

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

	Material material;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Load a model from file
	void loadModel(std::string modelPath);

	// Create a buffer in GPU memory and fill it with the received data
	void createBuffer(CommandManager commandManager, VkDeviceSize bufferSize, void* bufferData,
		VkBufferUsageFlagBits bufferUsage, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
};
