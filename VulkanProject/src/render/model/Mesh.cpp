#include "Mesh.hpp"

#include "asset/modelLoader.hpp"


namespace {
	// TODO: allocate more than one resource from a single call
	// TODO: store all the data in a single buffer and use offsets in calls with them
	void createBuffer(Device& device, CommandManager& commandManager, VkDeviceSize bufferSize,
		void* bufferData, VkBufferUsageFlagBits usage, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {

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
}

void Mesh::create(CommandManager& commandManager, std::string modelPath) {
	//--------------------------------------------------------
	// LOAD THE GEOMETRY
	loadMesh(modelPath);

	//--------------------------------------------------------
	// CREATE VERTEX AND INDEX BUFFERS
	createBuffer(device, commandManager, sizeof(vertices[0]) * vertices.size(), vertices.data(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer, vertexBufferMemory);
	createBuffer(device, commandManager, sizeof(indices[0]) * indices.size(), indices.data(),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer, indexBufferMemory);
}

void Mesh::loadMesh(std::string modelPath) {
	ModelData data = loadModelFromFile(modelPath);
	vertices = std::move(data.vertices);
	indices = std::move(data.indices);
}

void Mesh::cleanup() {
	vkDestroyBuffer(device.get(), vertexBuffer, nullptr);
	vertexBuffer = VK_NULL_HANDLE;
	vkFreeMemory(device.get(), vertexBufferMemory, nullptr);
	vertexBufferMemory = VK_NULL_HANDLE;
	vkDestroyBuffer(device.get(), indexBuffer, nullptr);
	indexBuffer = VK_NULL_HANDLE;
	vkFreeMemory(device.get(), indexBufferMemory, nullptr);
	indexBufferMemory = VK_NULL_HANDLE;
}
