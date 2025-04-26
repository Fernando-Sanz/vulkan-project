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

	size_t getId() const { return id; }

	const std::vector<uint32_t>& getIndices() const { return indices; }

	VkBuffer getIndexBuffer() const { return indexBuffer; }
	VkBuffer getVertexBuffer() const { return vertexBuffer; }

	Mesh(size_t id, Device& device, CommandManager commandManager, std::string modelPath) :
		id(id), device(device) {
		create(commandManager, modelPath);
	}

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	Device& device;

	const size_t id;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS
	
	friend class MeshManager;

	void create(CommandManager& commandManager, std::string modelPath);

	void loadMesh(std::string modelPath);

	void cleanup();
};

