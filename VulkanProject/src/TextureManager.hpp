#pragma once 

#include <vulkan/vulkan.h>

#include <string>

#include "Device.hpp"
#include "CommandManager.hpp"
#include "imageUtils.hpp"


struct TexturePaths {
	std::optional<std::string> albedoPath;
	std::optional<std::string> specularPath;
	std::optional<std::string> normalPath;
	std::vector<std::string> customPaths;
};

enum TextureType {
	TEXTURE_TYPE_ALBEDO_BIT = 0b0001,
	TEXTURE_TYPE_SPECULAR_BIT = 0b0010,
	TEXTURE_TYPE_NORMAL_BIT = 0b0100,
	TEXTURE_TYPE_CUSTOM_BIT = 0b1000
};

class TextureManager {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	int getTextureTypesUsed() { return usedTypes; }

	ImageObjects getAlbedo() { return albedo; }
	ImageObjects getSpecular() { return specular; }
	ImageObjects getNormal() { return normal; }

	ImageObjects getCustomTexture(size_t index) { return customTextures[index]; }
	std::vector<ImageObjects> getCustomTextures() { return customTextures; }

	uint32_t getTextureCount() { return textureCount; }

	VkSampler getSampler() { return sampler; }


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS
	
	// Store the device and command manager for future operations
	void create(Device device, CommandManager commandManager);

	// Add a texture of the specified type
	void addTexture(TextureType type, ImageObjects texture);

	// Create all the resources involved in texture usage and add it to the texture list
	void createTexture(TextureType type, std::string texturePath);

	// Create all textures with a defined path in the struct texturePaths
	void TextureManager::createTextures(TexturePaths texturePaths);

	// Destroy Vulkan and other objects
	void cleanup(bool destroyVulkanImages = true);

	// Erase the texture handlers and destroy the Vulkan objects associated if it is specified
	void destroyTextures(bool destroyVulkanImages = true);

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	Device device;
	CommandManager commandManager;

	uint32_t textureCount = 0;
	int usedTypes = 0;
	ImageObjects albedo;
	ImageObjects specular;
	ImageObjects normal;
	std::vector<ImageObjects> customTextures;
	uint32_t mipLevels = 1; // computed in createTexture() since them depend on the texture size
	VkSampler sampler = VK_NULL_HANDLE;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Create an image and its memory for the texture and generate its mipmaps
	void createTextureImage(std::string texturePath, ImageObjects& texture);
	
	// Create sampler for the textures
	void createSampler(uint32_t mipLevels);
};


