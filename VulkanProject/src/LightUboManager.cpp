#include "LightUboManager.hpp"

#include <stdexcept>


void LightUboManager::createBuffers(Device device, size_t count, size_t lightCount) {

	//--------------------------------------------------------
	// SET CLASS MEMBERS

	this->device = device;

	// uniforms per FRAME IN FLIGHT
	buffers.resize(count);
	for (auto& uboBuffers : buffers) {
		// uniform resources per LIGHT
		uboBuffers.buffers.resize(lightCount);
		uboBuffers.memory.resize(lightCount);
		uboBuffers.mapped.resize(lightCount);
	}

	//--------------------------------------------------------
	// CREATE BUFFERS

	VkDeviceSize bufferSize = sizeof(LightUBO);

	// uniforms per FRAME IN FLIGHT
	for (auto& uboBuffers : buffers) {
		// uniform resources per LIGHT
		for (size_t i = 0; i < uboBuffers.buffers.size(); i++) {

			device.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uboBuffers.buffers[i], uboBuffers.memory[i]);

			vkMapMemory(device.get(), uboBuffers.memory[i], 0, bufferSize, 0, &uboBuffers.mapped[i]);
		}
	}
}

void LightUboManager::upateBuffers(uint32_t index, std::vector<Light> lights, Camera camera) {
	
	UBOResources uniforms = buffers[index];
	if (lights.size() != uniforms.buffers.size()) {
		throw std::runtime_error("tried to update invalid light count");
	}

	LightUBO ubo{};
	glm::mat4 view = camera.getView();

	// uniform per LIGHT
	for (size_t i = 0; i < uniforms.buffers.size(); i++) {
		updateBuffer(uniforms.mapped[i], lights[index], view);
	}
}

void LightUboManager::updateBuffer(void* mapped, Light light, glm::mat4 view) {

	LightUBO ubo{};

	// Light pos and dir in camera coordinates
	glm::vec4 lightPos = view * glm::vec4(light.getPosition(), 1.0f);
	glm::vec4 lightDirection = view * glm::vec4(light.getDirection(), 0.0f);
	ubo.pos = glm::vec3(lightPos);
	ubo.color = light.getColor();
	ubo.direction = glm::vec3(lightDirection);

	memcpy(mapped, &ubo, sizeof(ubo));
}

void LightUboManager::cleanup() {
	// uniforms per FRAME IN FLIGHT
	for(auto& uboBuffers : buffers) {
		// uniform resources per LIGHT
		for (size_t i = 0; i < uboBuffers.buffers.size(); i++) {
			vkDestroyBuffer(device.get(), uboBuffers.buffers[i], nullptr);
			vkFreeMemory(device.get(), uboBuffers.memory[i], nullptr);
		}
	}
}
