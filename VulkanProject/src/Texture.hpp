#pragma once 

#include <vulkan/vulkan.h>

#include <string>

#include "Device.hpp"
#include "CommandManager.hpp"


// TODO: check if it could be in Model
class Texture {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	VkImage getImage() { return image; }
	VkImageView getImageView() { return imageView; }
	VkSampler getSampler() { return sampler; }
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Create all the resources involved in texture usage
	void create(Device device, CommandManager commandManager, std::string texturePath);

	// Create sampler for the texture
	static void createSampler(Device device, uint32_t mipLevels, VkSampler& sampler);

	// Destroy Vulkan and other objects
	void cleanup();

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	Device device;
	CommandManager commandManager;

	uint32_t mipLevels;
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
	VkSampler sampler;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Create an image and its memory for the texture and generate its mipmaps
	void createTextureImage(std::string texturePath);

};
