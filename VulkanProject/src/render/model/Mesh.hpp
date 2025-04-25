#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string>

#include "context/Device.hpp"
#include "context/CommandManager.hpp"
#include "render/model/Vertex.hpp"


struct Mesh {

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// MEMBERS

	const size_t id;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	Mesh(size_t id) : id(id) {}
};

void destroyMesh(Device device, Mesh& mesh);
