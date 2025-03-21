#include "Texture.hpp"

#include <iostream>
#include <stdexcept>
#include <stb_image.h>

#include "ImageUtils.hpp"


void Texture::create(Device device, CommandManager commandManager, std::string texturePath) {
	this->device = device;
	this->commandManager = commandManager;

	createTextureImage(texturePath);
	imageView = createImageView(
		device, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
	createTextureSampler();
}

void Texture::createTextureImage(std::string texturePath) {
	//-----------------------------------------
	// LOAD IMAGE
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels) throw std::runtime_error("failed to load texture image");

	VkDeviceSize imageSize = texWidth * texHeight * 4/*bytes per pixel*/;
	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	std::cout << "Loaded texture ";
	std::cout << texWidth << "x" << texHeight << std::endl;

	//-----------------------------------------
	// COPY IMAGE TO STAGING BUFFER
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device.get(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device.get(), stagingBufferMemory);

	// cleanup old pixel array
	stbi_image_free(pixels);

	//-----------------------------------------
	// CREATE IMAGE
	createImage(device, texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT,
		VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		image, imageMemory);

	//-----------------------------------------
	// COPY THE IMAGE FROM STAGING BUFFER
	transitionImageLayout(commandManager, image,
		VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		mipLevels);
	copyBufferToImage(commandManager, stagingBuffer, image,
		static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	// transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

	//-----------------------------------------
	// CLEAN UP STAGING STUFF
	vkDestroyBuffer(device.get(), stagingBuffer, nullptr);
	vkFreeMemory(device.get(), stagingBufferMemory, nullptr);

	//-----------------------------------------
	// GENERATE MIPMAPS
	generateMipmaps(device, commandManager, image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
}

void Texture::createTextureSampler() {
	//-----------------------------------------
	// CREATE INFO
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	// ANISOTROPIC FILTERING
	VkPhysicalDeviceProperties properties = device.getPhysicalDeviceProperties();
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	// COMPARE OPS. (for shadow mapping and similar)
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	// MIPMAPS
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f; // optional
	samplerInfo.minLod = 0.0f; // optional
	samplerInfo.maxLod = static_cast<float>(mipLevels);

	// CREATE SAMPLER
	if (vkCreateSampler(device.get(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler");
	}
}

void Texture::cleanup() {
	vkDestroySampler(device.get(), sampler, nullptr);
	vkDestroyImageView(device.get(), imageView, nullptr);
	vkDestroyImage(device.get(), image, nullptr);
	vkFreeMemory(device.get(), imageMemory, nullptr);
}
