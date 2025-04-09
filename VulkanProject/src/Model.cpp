#include "Model.hpp"

#include <iostream>
#include <tiny_obj_loader.h>

#include "AppTime.hpp"


void Model::loadModel(Device device, CommandManager commandManager, std::string modelPath) {

	this->device = device;

	//--------------------------------------------------------
	// Load obj file

	tinyobj::ObjReader reader;
	tinyobj::ObjReaderConfig reader_config;

	if (!reader.ParseFromFile(modelPath, reader_config)) {
		if (!reader.Error().empty()) {
			throw std::runtime_error(reader.Error());
		}
	}

	if (!reader.Warning().empty()) {
		std::cout << "TinyObjLoader: " << reader.Warning();
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();

	//--------------------------------------------------------
	// Fill in the model with the loaded data
	
	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	// SHAPES LOOP
	for (const auto& shape : shapes) {
		// FACES LOOP
		for (const auto& index : shape.mesh.indices) {

			Vertex vertex{};

			//-------------------------
			// POSITION
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			//-------------------------
			// NORMAL
			if (index.normal_index >= 0) {
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
			}

			//-------------------------
			// TEXTURE COORDINATES
			if (index.texcoord_index >= 0) {
				// flip vertical component for correct visualization (OBJ to Vulkan conversion)
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}

			//-------------------------
			// check if the vertex already exists and store the index
			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}

	std::cout << "Model loaded: " << vertices.size() << " vertices" << std::endl;

	//--------------------------------------------------------
	// CREATE VERTEX AND INDEX BUFFERS
	createModelBuffer(commandManager, sizeof(vertices[0]) * vertices.size(), vertices.data(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer, vertexBufferMemory);
	createModelBuffer(commandManager, sizeof(indices[0]) * indices.size(), indices.data(),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer, indexBufferMemory);
}

// TODO: allocate more than one resource from a single call
// TODO: store all the data in a single buffer and use offsets in calls with them
void Model::createModelBuffer(CommandManager commandManager, VkDeviceSize bufferSize, void* bufferData,
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

void Model::update() {
	static float angle = 0.0f;
	angle += AppTime::deltaTime() * glm::radians(90.0f);
	model = glm::rotate(
		glm::mat4(1.0f), angle * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}

void Model::cleanup() {
	vkDestroyBuffer(device.get(), vertexBuffer, nullptr);
	vkFreeMemory(device.get(), vertexBufferMemory, nullptr);
	vkDestroyBuffer(device.get(), indexBuffer, nullptr);
	vkFreeMemory(device.get(), indexBufferMemory, nullptr);
}
