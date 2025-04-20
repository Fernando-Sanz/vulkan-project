#include "Model.hpp"

#include <iostream>
#include "asset/modelLoader.hpp"


void Model::create(Device device, CommandManager commandManager, std::string modelPath, Material material,
	bool useRawVertexData) {

	this->device = device;
	this->material = material;

	//--------------------------------------------------------
	// LOAD THE GEOMETRY
	loadModel(modelPath);

	//--------------------------------------------------------
	// CREATE VERTEX AND INDEX BUFFERS
	createBuffer(commandManager, sizeof(vertices[0]) * vertices.size(), vertices.data(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer, vertexBufferMemory);
	createBuffer(commandManager, sizeof(indices[0]) * indices.size(), indices.data(),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer, indexBufferMemory);

	//--------------------------------------------------------
	// TRANSFORM (if the vertex data will be used with transformations)
	if (!useRawVertexData) {
		this->transform = new Transform();
		this->transform->position = glm::vec3(0.0f, 0.0f, -0.15);
		this->transform->scale = glm::vec3(1.2f);
	}
}

void Model::loadModel(std::string modelPath) {
	ModelData data = loadModelFromFile(modelPath);
	vertices = std::move(data.vertices);
	indices = std::move(data.indices);
}

// TODO: allocate more than one resource from a single call
// TODO: store all the data in a single buffer and use offsets in calls with them
void Model::createBuffer(CommandManager commandManager, VkDeviceSize bufferSize, void* bufferData,
	VkBufferUsageFlagBits usage, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {

	//--------------------------------------------
	// STAGING BUFFER
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	// copy data to the buffer
	void* data;
	vkMapMemory(device.get(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, bufferData, (size_t)bufferSize);
	vkUnmapMemory(device.get(), stagingBufferMemory);

	//--------------------------------------------
	// CREATE BUFFER AND FILL IT WITH DATA
	device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);

	commandManager.copyBuffer(stagingBuffer, buffer, bufferSize);

	//--------------------------------------------
	// Clean up
	vkDestroyBuffer(device.get(), stagingBuffer, nullptr);
	vkFreeMemory(device.get(), stagingBufferMemory, nullptr);
}

void Model::cleanup() {
	vkDestroyBuffer(device.get(), vertexBuffer, nullptr);
	vkFreeMemory(device.get(), vertexBufferMemory, nullptr);
	vkDestroyBuffer(device.get(), indexBuffer, nullptr);
	vkFreeMemory(device.get(), indexBufferMemory, nullptr);
	material.cleanup();
}
