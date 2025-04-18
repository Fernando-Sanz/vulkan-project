#include "asset/pipelineUtils.hpp"

#include <stdexcept>
#include <iostream>
#include <fstream>


std::vector<char> readFile(const std::string& filename) {

	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file");
	}

	// take the size
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	std::cout << "Shader " << filename << " with size: " << fileSize << std::endl;

	// read
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

VkShaderModule createShaderModule(Device device, const std::vector<char>& code) {
	// CREATE INFO
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size(); // Size in bytes (char)
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); // Vulkan uses uint32 pointer (not char)
	// It is not necessary to check alignment, std::vector already does

	// CREATE SHADER MODULE
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device.get(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module");
	}

	return shaderModule;
}
