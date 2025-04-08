#pragma once 

#include <vulkan/vulkan.h>

#include <string>

#include "Device.hpp"
#include "CommandManager.hpp"
#include "imageUtils.hpp"


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

	VkSampler getSampler() { return sampler; }


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS
	
	// Store the device and command manager for future operations
	void create(Device device, CommandManager commandManager);

	// Add a texture of the specified type
	void addTexture(TextureType type, ImageObjects texture);

	// Create all the resources involved in texture usage and add it to the texture list
	void createTexture(TextureType type, std::string texturePath);

	// Destroy the specified texture, if type = TEXTURE_TYPE_CUSTOM_BIT only first custom texture is destroyed
	void destroyTexture(TextureType type);

	// Destroy the specified custom texture
	void destroyCustomTexture(size_t index);

	// Destroy Vulkan and other objects
	void cleanup();

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	Device device;
	CommandManager commandManager;

	int usedTypes = 0;
	ImageObjects albedo;
	ImageObjects specular;
	ImageObjects normal;
	std::vector<ImageObjects> customTextures;
	uint32_t mipLevels = 1;
	VkSampler sampler = VK_NULL_HANDLE;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Create an image and its memory for the texture and generate its mipmaps
	void createTextureImage(std::string texturePath, ImageObjects& texture);
	
	// Create sampler for the textures
	void createSampler(uint32_t mipLevels);
};


