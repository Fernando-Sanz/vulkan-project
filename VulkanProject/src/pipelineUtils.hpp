#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>


std::vector<char> readFile(const std::string& filename);

VkShaderModule createShaderModule(const std::vector<char>& code);
