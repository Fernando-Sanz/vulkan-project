#pragma once 

#include <vulkan/vulkan.h>

#include <string>

#include "context/Device.hpp"
#include "context/CommandManager.hpp"
#include "render/image/imageUtils.hpp"


struct TexturePaths {
	std::optional<std::string> albedoPath;
	std::optional<std::string> specularPath;
	std::optional<std::string> normalPath;
	std::vector<std::string> customPaths;
};

enum TextureType {
	TEXTURE_TYPE_NONE_BIT = 0b0000,
	TEXTURE_TYPE_ALBEDO_BIT = 0b0001,
	TEXTURE_TYPE_SPECULAR_BIT = 0b0010,
	TEXTURE_TYPE_NORMAL_BIT = 0b0100,
	TEXTURE_TYPE_CUSTOM_BIT = 0b1000
};


// TODO: add other material info (shininess, etc)
struct Material {

	using TextureTypes = uint32_t;

	Device device;
	CommandManager commandManager;

	size_t textureCount = 0;
	TextureTypes usedTypes = TEXTURE_TYPE_NONE_BIT;

	ImageObjects albedoTexture;
	ImageObjects specularTexture;
	ImageObjects normalTexture;
	std::vector<ImageObjects> customTextures;

	uint32_t mipLevels = 1; // computed in createTexture() since them depend on the texture size
	// TODO: review other owner for the sampler
	VkSampler sampler = VK_NULL_HANDLE;

	bool hasAlbedo() const { return usedTypes & TEXTURE_TYPE_ALBEDO_BIT; }
	bool hasSpecular() const { return usedTypes & TEXTURE_TYPE_SPECULAR_BIT; }
	bool hasNormal() const { return usedTypes & TEXTURE_TYPE_NORMAL_BIT; }
	bool hasCustom() const { return usedTypes & TEXTURE_TYPE_CUSTOM_BIT; }
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS
	
	// Store the device and command manager for future operations
	void setContext(Device device, CommandManager commandManager);

	// Create all textures with a defined path in the struct texturePaths
	void createTextures(TexturePaths texturePaths);

	// Create all the resources involved in texture usage and add it to the texture list
	void createTexture(TextureType type, std::string texturePath);

	// Add a texture of the specified type
	void addTexture(TextureType type, ImageObjects texture);

	// Destroy Vulkan and other objects
	void cleanup();

	// Erase the texture handlers and destroy the Vulkan objects associated if it is specified
	void destroyTextures();

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Create an image and its memory for the texture and generate its mipmaps
	void createTextureImage(std::string texturePath, ImageObjects& texture);
	
	// Create sampler for the textures
	void createSampler(uint32_t mipLevels);

	void storeTexture(TextureType type, ImageObjects texture);
};
