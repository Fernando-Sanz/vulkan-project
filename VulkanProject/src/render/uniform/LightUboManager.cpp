#include "render/uniform/LightUboManager.hpp"


void LightUboManager::createBuffers(Device device, size_t count, size_t lightCount) {

	//--------------------------------------------------------
	// SET CLASS MEMBERS

	this->device = device;
	this->lightCount = lightCount;

	buffers.resize(count);
	buffersMemory.resize(count);
	buffersMapped.resize(count);

	//--------------------------------------------------------
	// CREATE BUFFERS

	VkDeviceSize bufferSize = sizeof(LightUBO) * lightCount;
	for (size_t i = 0; i < count; i++) {
		device.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			buffers[i], buffersMemory[i]);

		vkMapMemory(device.get(), buffersMemory[i], 0, bufferSize, 0, &buffersMapped[i]);
	}
}

void LightUboManager::upateBuffer(uint32_t index, std::vector<Light> lights, Camera camera) {
	
	glm::mat4 view = camera.getView();

	std::vector<LightUBO> lightUBOs(lights.size());
	for (size_t i = 0; i < lights.size(); i++) {
		LightUBO ubo{};

		// Light pos and dir in camera coordinates
		glm::vec4 lightPos = view * glm::vec4(lights[i].getPosition(), 1.0f);
		glm::vec4 lightDirection = view * glm::vec4(lights[i].getDirection(), 0.0f);
		ubo.pos = glm::vec3(lightPos);
		ubo.color = lights[i].getColor();
		ubo.direction = glm::vec3(lightDirection);

		lightUBOs[i] = ubo;
	}

	memcpy(buffersMapped[index], lightUBOs.data(), sizeof(LightUBO) * lights.size());
}

void LightUboManager::cleanup() {
	for (size_t i = 0; i < buffers.size(); i++) {
		vkDestroyBuffer(device.get(), buffers[i], nullptr);
		vkFreeMemory(device.get(), buffersMemory[i], nullptr);
	}
}
