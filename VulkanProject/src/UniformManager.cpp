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
}

void UniformManager::upateBuffer(uint32_t index, uint32_t screenWidth, uint32_t screenHeight, glm::mat4 model) {
	//--------------------------------------------------------
	// UNIFORM VALUES

	UniformBufferObject ubo{};

	//-------------------------
	// MODEL
	ubo.model = model;
	//-------------------------
	// VIEW
	ubo.view = glm::lookAt(
		glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//-------------------------
	// PROJECTION
	ubo.proj = glm::perspective(
		glm::radians(45.0f), screenWidth / (float)screenHeight, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1; // non-OpenGL GLM usage adjustment

	//--------------------------------------------------------
	// UPDATE BUFFER

	memcpy(buffersMapped[index], &ubo, sizeof(ubo));
}

void UniformManager::cleanup() {
	size_t count = buffers.size();
	for (size_t i = 0; i < count; i++) {
		vkDestroyBuffer(device.get(), buffers[i], nullptr);
		vkFreeMemory(device.get(), buffersMemory[i], nullptr);
	}
}
