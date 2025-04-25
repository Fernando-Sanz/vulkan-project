#include "render/uniform/Material.hpp"

#include <iostream>
#include <stdexcept>
#include <stb_image.h>

#include "asset/imageLoader.hpp"
#include "render/image/imageUtils.hpp"


void Material::setContext(Device device, CommandManager commandManager) {
	this->device = device;
	this->commandManager = commandManager;
}

void Material::createTextures(TexturePaths texturePaths) {

	if (texturePaths.albedoPath.has_value())
		createTexture(TEXTURE_TYPE_ALBEDO_BIT, texturePaths.albedoPath.value());
	if (texturePaths.specularPath.has_value())
		createTexture(TEXTURE_TYPE_SPECULAR_BIT, texturePaths.specularPath.value());
	if (texturePaths.normalPath.has_value())
		createTexture(TEXTURE_TYPE_NORMAL_BIT, texturePaths.normalPath.value());
	for (auto& texturePath : texturePaths.customPaths)
		createTexture(TEXTURE_TYPE_CUSTOM_BIT, texturePath);
}

void Material::createTexture(TextureType type, std::string texturePath) {
	// CREATE THE TEXTURE
	ImageObjects newTexture{};
	createTextureImage(texturePath, newTexture);
	newTexture.view = createImageView(
		device, newTexture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

	storeTexture(type, newTexture);
}

void Material::addTexture(TextureType type, ImageObjects texture) {
	texture.ownedImage = false;
	storeTexture(type, texture);
}

void Material::storeTexture(TextureType type, ImageObjects texture) {
	// CREATE THE SAMPLER (if not created yet)
	if (sampler == VK_NULL_HANDLE) createSampler(mipLevels);

	// STORE THE TEXTURE
	switch (type) {
	case TEXTURE_TYPE_ALBEDO_BIT:
		albedoTexture = texture;
		break;
	case TEXTURE_TYPE_SPECULAR_BIT:
		specularTexture = texture;
		break;
	case TEXTURE_TYPE_NORMAL_BIT:
		normalTexture = texture;
		break;
	case TEXTURE_TYPE_CUSTOM_BIT:
		customTextures.push_back(texture);
		break;
	default:
		throw std::runtime_error("texture type not valid");
	}

	// Store the type
	usedTypes |= type;
	textureCount++;
}

void Material::createSampler(uint32_t mipLevels) {
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

// TODO: review if this method or part of it could be in imageUtils or similar
void Material::createTextureImage(std::string texturePath, ImageObjects& texture) {
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


void Material::cleanup() {
	vkDestroySampler(device.get(), sampler, nullptr);

	destroyTextures();
}

void Material::destroyTextures() {

	if (TEXTURE_TYPE_ALBEDO_BIT & usedTypes)
		destroyImageObjects(device, albedoTexture);
	if (TEXTURE_TYPE_SPECULAR_BIT & usedTypes)
		destroyImageObjects(device, specularTexture);
	if (TEXTURE_TYPE_NORMAL_BIT & usedTypes)
		destroyImageObjects(device, normalTexture);

	for (auto& texture : customTextures) {
		destroyImageObjects(device, texture);
	}
	customTextures.clear();

	textureCount = 0;
	usedTypes = TEXTURE_TYPE_NONE_BIT;
}
