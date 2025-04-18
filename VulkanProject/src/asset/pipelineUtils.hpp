#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "context/Device.hpp"


std::vector<char> readFile(const std::string& filename);

VkShaderModule createShaderModule(Device device, const std::vector<char>& code);
