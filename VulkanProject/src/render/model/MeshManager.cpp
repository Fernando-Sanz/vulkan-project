#include "render/model/MeshManager.hpp"

#include "asset/modelLoader.hpp"


static void loadMesh(std::string modelPath, Mesh* mesh) {
	ModelData data = loadModelFromFile(modelPath);
	mesh->vertices = std::move(data.vertices);
	mesh->indices = std::move(data.indices);
}

void MeshManager::init(Device device, CommandManager commandManager) {
	this->device = device;
	this->commandManager = commandManager;
}

Mesh* MeshManager::createMesh(std::string modelPath) {

	auto [iterator, inserted] = meshes.insert({ nextId, std::make_unique<Mesh>(nextId) });
	nextId++;
	Mesh* mesh = iterator->second.get();

	//--------------------------------------------------------
	// LOAD THE GEOMETRY
	loadMesh(modelPath, mesh);

	//--------------------------------------------------------
	// CREATE VERTEX AND INDEX BUFFERS
	createBuffer(sizeof(mesh->vertices[0]) * mesh->vertices.size(), mesh->vertices.data(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mesh->vertexBuffer, mesh->vertexBufferMemory);
	createBuffer(sizeof(mesh->indices[0]) * mesh->indices.size(), mesh->indices.data(),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mesh->indexBuffer, mesh->indexBufferMemory);

	return mesh;
}

// TODO: allocate more than one resource from a single call
// TODO: store all the data in a single buffer and use offsets in calls with them
void MeshManager::createBuffer(VkDeviceSize bufferSize, void* bufferData, VkBufferUsageFlagBits usage,
	VkBuffer& buffer, VkDeviceMemory& bufferMemory) {

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

void MeshManager::deleteMesh(size_t id) {
	if (!meshes.count(id)) return;

	destroyMesh(device, *meshes[id].get());
	meshes.erase(id);
}

void MeshManager::cleanup() {
	for (const auto& pair : meshes) {
		deleteMesh(pair.first);
	}
}
