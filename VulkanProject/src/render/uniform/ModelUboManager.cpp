#include "render/uniform/ModelUboManager.hpp"


void ModelUboManager::createBuffers(Device device, int count) {

	//--------------------------------------------------------
	// SET CLASS MEMBERS

	this->device = device;

	buffers.resize(count);
	buffersMemory.resize(count);
	buffersMapped.resize(count);

	//--------------------------------------------------------
	// CREATE BUFFERS

	VkDeviceSize bufferSize = sizeof(ModelUBO);
	for (size_t i = 0; i < count; i++) {
		device.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			buffers[i], buffersMemory[i]);

		vkMapMemory(device.get(), buffersMemory[i], 0, bufferSize, 0, &buffersMapped[i]);
	}
}

void ModelUboManager::upateBuffer(uint32_t index, Model model, Camera camera) {
	
	ModelUBO ubo{};
	glm::mat4 view = camera.getView();

	// Compute matrices
	ubo.modelView = view * createModelMatrix(model.getTransform());
	ubo.invTrans_modelView = glm::inverse(glm::transpose(ubo.modelView));
	ubo.proj = camera.getProjection();

	memcpy(buffersMapped[index], &ubo, sizeof(ubo));
}

void ModelUboManager::cleanup() {
	for (size_t i = 0; i < buffers.size(); i++) {
		vkDestroyBuffer(device.get(), buffers[i], nullptr);
		vkFreeMemory(device.get(), buffersMemory[i], nullptr);
	}
}
