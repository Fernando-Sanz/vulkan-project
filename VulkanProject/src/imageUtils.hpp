#pragma once

#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "CommandManager.hpp"

	
// Create an image with the specified properties and bind it with the memory
void createImage(Device device, uint32_t width, uint32_t height, uint32_t mipLevels,
	VkSampleCountFlagBits sampleCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

// Create the image view of the image with the specified info
VkImageView createImageView(Device device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
	uint32_t mipLevels);

// Copy the content of a buffer to an image
void copyBufferToImage(CommandManager commandManager, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

// Make transition of image layout to another specified
void transitionImageLayout(CommandManager commandManager, VkImage image, VkFormat format,
	VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

// Generate 'mipLevels' mipmaps of the image
void generateMipmaps(Device device, CommandManager commandManager,
	VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
