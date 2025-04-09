#include "UniformManager.hpp"

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // range 0.0 to 1.0 not -1.0 to 1.0
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Device.hpp"
#include "AppTime.hpp"


void UniformManager::createBuffers(Device device, int count) {

	//--------------------------------------------------------
	// SET CLASS MEMBERS

	this->device = device;

	buffers.resize(count);
	buffersMemory.resize(count);
	buffersMapped.resize(count);

	//--------------------------------------------------------
	// CREATE BUFFERS

	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	for (size_t i = 0; i < count; i++) {
		device.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			buffers[i], buffersMemory[i]);

		vkMapMemory(device.get(), buffersMemory[i], 0, bufferSize, 0, &buffersMapped[i]);
	}

	bufferSize = sizeof(LightUBO);
	device.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		lightUBOBuffer, lightUBOMemory);

	vkMapMemory(device.get(), lightUBOMemory, 0, bufferSize, 0, &lightUBOMapped);
}

void UniformManager::upateBuffer(uint32_t index, glm::mat4 model, Camera camera) {
	//--------------------------------------------------------
	// UNIFORM VALUES

	UniformBufferObject ubo{};

	//-------------------------
	// MODEL
	ubo.model = model;
	//-------------------------
	// CAMERA VARIABLES
	ubo.view = camera.getView();
	ubo.proj = camera.getProjection();

	//-------------------------
	// LIGHT VARIABLES
	LightUBO fragUBO{};

	// Light pos and dir in camera coordinates
	glm::vec4 lightPos = ubo.view * glm::vec4(0.0f, 2.0f, 3.0f, 1.0f);
	glm::vec4 lightDirection = ubo.view * glm::vec4(1.0f, -1.0f, 0.0f, 0.0f);
	fragUBO.lightPos = glm::vec3(lightPos);
	fragUBO.lightColor = glm::vec3(0.5f, 0.437f, 0.365f);
	fragUBO.lightDirection = glm::vec3(lightDirection);

	//--------------------------------------------------------
	// UPDATE BUFFER

	memcpy(buffersMapped[index], &ubo, sizeof(ubo));
	memcpy(lightUBOMapped, &fragUBO, sizeof(LightUBO));
}

void UniformManager::cleanup() {
	size_t count = buffers.size();
	for (size_t i = 0; i < count; i++) {
		vkDestroyBuffer(device.get(), buffers[i], nullptr);
		vkFreeMemory(device.get(), buffersMemory[i], nullptr);
	}
	vkDestroyBuffer(device.get(), lightUBOBuffer, nullptr);
	vkFreeMemory(device.get(), lightUBOMemory, nullptr);
}
