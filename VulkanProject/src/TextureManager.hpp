#pragma once 

#include <vulkan/vulkan.h>

#include <string>

#include "Device.hpp"
#include "CommandManager.hpp"
#include "imageUtils.hpp"


class TextureManager {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	ImageObjects getTexture(size_t index) { return textures[index]; }
	std::vector<ImageObjects> getTextures() { return textures; }
	VkSampler getSampler() { return sampler; }


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS
	
	// Store the device and command manager for future operations
	void create(Device device, CommandManager commandManager);

	// Add a texture to the list
	void addTexture(ImageObjects texture);

	// Create all the resources involved in texture usage and add it to the texture list
	void createTexture(std::string texturePath);

	// Destroy the specified texture
	void destroyTexture(size_t index);

	// Destroy Vulkan and other objects
	void cleanup();

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	Device device;
	CommandManager commandManager;

	std::vector<ImageObjects> textures;
	uint32_t mipLevels = 1;
	VkSampler sampler = VK_NULL_HANDLE;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Create an image and its memory for the texture and generate its mipmaps
	void createTextureImage(std::string texturePath, ImageObjects& texture);
	
	// Create sampler for the textures
	void createSampler(uint32_t mipLevels);
};


