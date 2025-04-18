#include "render/uniform/TextureManager.hpp"

#include <iostream>
#include <stdexcept>
#include <stb_image.h>

#include "asset/imageLoader.hpp"
#include "render/image/imageUtils.hpp"


void TextureManager::create(Device device, CommandManager commandManager) {
	this->device = device;
	this->commandManager = commandManager;
}

void TextureManager::addTexture(TextureType type, ImageObjects texture) {
	// CREATE THE SAMPLER (if not created yet)
	if (sampler == VK_NULL_HANDLE) createSampler(mipLevels);

	// STORE THE TEXTURE
	switch (type) {
	case TEXTURE_TYPE_ALBEDO_BIT:
		albedo = texture;
		break;
	case TEXTURE_TYPE_SPECULAR_BIT:
		specular = texture;
		break;
	case TEXTURE_TYPE_NORMAL_BIT:
		normal = texture;
		break;
	case TEXTURE_TYPE_CUSTOM_BIT:
		customTextures.push_back(texture);
		break;
	default:
		throw std::runtime_error("texture type not supported");
	}

	// Store the type
	usedTypes |= type;
	textureCount++;
}

void TextureManager::createTexture(TextureType type, std::string texturePath) {
	// CREATE THE TEXTURE
	ImageObjects newTexture{};
	createTextureImage(texturePath, newTexture);
	newTexture.view = createImageView(
		device, newTexture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

	addTexture(type, newTexture);
}

void TextureManager::createTextures(TexturePaths texturePaths) {

	if (texturePaths.albedoPath.has_value())
		createTexture(TEXTURE_TYPE_ALBEDO_BIT, texturePaths.albedoPath.value());
	if (texturePaths.specularPath.has_value())
		createTexture(TEXTURE_TYPE_SPECULAR_BIT, texturePaths.specularPath.value());
	if (texturePaths.normalPath.has_value())
		createTexture(TEXTURE_TYPE_NORMAL_BIT, texturePaths.normalPath.value());
	for (auto& texturePath : texturePaths.customPaths)
		createTexture(TEXTURE_TYPE_CUSTOM_BIT, texturePath);
}

void TextureManager::createTextureImage(std::string texturePath, ImageObjects& texture) {
	//-----------------------------------------
	// LOAD IMAGE
	RawImage image = loadImageFromFile(texturePath);
	VkDeviceSize imageSize = static_cast<VkDeviceSize>(image.size);
	uint32_t texWidth = static_cast<uint32_t>(image.width);
	uint32_t texHeight = static_cast<uint32_t>(image.height);
	// Compute mip levels
	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(image.width, image.height)))) + 1;

	//-----------------------------------------
	// COPY IMAGE TO STAGING BUFFER
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device.get(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, image.pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device.get(), stagingBufferMemory);

	// cleanup old pixel array
	image.free();

	//-----------------------------------------
	// CREATE IMAGE
	createImage(device, texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT,
		VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		texture.image, texture.memory);

	//-----------------------------------------
	// COPY THE IMAGE FROM STAGING BUFFER
	transitionImageLayout(commandManager, texture.image,
		VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		mipLevels);
	copyBufferToImage(commandManager, stagingBuffer, texture.image,
		static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	// transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

	//-----------------------------------------
	// CLEAN UP STAGING STUFF
	vkDestroyBuffer(device.get(), stagingBuffer, nullptr);
	vkFreeMemory(device.get(), stagingBufferMemory, nullptr);

	//-----------------------------------------
	// GENERATE MIPMAPS
	generateMipmaps(device, commandManager, texture.image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
}

void TextureManager::createSampler(uint32_t mipLevels) {
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

void TextureManager::destroyTextures(bool destroyVulkanImages) {

	if (destroyVulkanImages) {
		if (TEXTURE_TYPE_ALBEDO_BIT & usedTypes)
			destroyImageObjects(device, albedo);
		if (TEXTURE_TYPE_SPECULAR_BIT & usedTypes)
			destroyImageObjects(device, specular);
		if (TEXTURE_TYPE_NORMAL_BIT & usedTypes)
			destroyImageObjects(device, normal);

		for (auto& texture : customTextures) {
			destroyImageObjects(device, texture);
		}
	}
	customTextures.clear();

	textureCount = 0;
	usedTypes = 0b0000;
}

void TextureManager::cleanup(bool destroyVulkanImages) {
	vkDestroySampler(device.get(), sampler, nullptr);

	destroyTextures(destroyVulkanImages);
}
