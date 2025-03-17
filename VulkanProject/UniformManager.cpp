#include "UniformManager.hpp"

#include <vulkan/vulkan.h>

#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // range 0.0 to 1.0 not -1.0 to 1.0
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Device.hpp"


void UniformManager::createBuffers(Device device, int count) {
	this->device = device;
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	buffers.resize(count);
	buffersMemory.resize(count);
	buffersMapped.resize(count);

	for (size_t i = 0; i < count; i++) {
		device.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			buffers[i], buffersMemory[i]);

		vkMapMemory(device.get(), buffersMemory[i], 0, bufferSize, 0, &buffersMapped[i]);
	}
}

// TODO: change this way and use push constants
void UniformManager::upateBuffer(uint32_t index, uint32_t screenWidth, uint32_t screenHeight) {
	// Time management
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	// UNIFORM BUFFER
	UniformBufferObject ubo{};

	// model
	ubo.model = glm::rotate(
		glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	// view
	ubo.view = glm::lookAt(
		glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	// proj
	ubo.proj = glm::perspective(
		glm::radians(45.0f), screenWidth / (float)screenHeight, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1; // non-OpenGL GLM usage adjustment

	// Copy data
	memcpy(buffersMapped[index], &ubo, sizeof(ubo));
}

void UniformManager::cleanup() {
	int count = buffers.size();
	for (size_t i = 0; i < count; i++) {
		vkDestroyBuffer(device.get(), buffers[i], nullptr);
		vkFreeMemory(device.get(), buffersMemory[i], nullptr);
	}
}
