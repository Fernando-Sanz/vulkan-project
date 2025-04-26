#pragma once 

#include <vulkan/vulkan.h>

#include <vector>
#include <string>

#include "context/Device.hpp"
#include "context/CommandManager.hpp"
#include "render/image/imageUtils.hpp"


// TODO: review if MaterialManager is a better place to place this
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
class Material {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	VkSampler getSampler() const { return sampler; }

	size_t getTextureCount() const { return textureCount; }

	bool hasAlbedo() const { return usedTypes & TEXTURE_TYPE_ALBEDO_BIT; }
	bool hasSpecular() const { return usedTypes & TEXTURE_TYPE_SPECULAR_BIT; }
	bool hasNormal() const { return usedTypes & TEXTURE_TYPE_NORMAL_BIT; }
	bool hasCustom() const { return usedTypes & TEXTURE_TYPE_CUSTOM_BIT; }

	ImageObjects getAlbedo() const { return albedoTexture; }
	ImageObjects getSpecular() const { return specularTexture; }
	ImageObjects getNormal() const { return normalTexture; }
	const std::vector<ImageObjects>& getCustomTextures() const { return customTextures; }

	Material(size_t id, Device& device, CommandManager commandManager, TexturePaths paths) :
		Material(id, device) {
		createTextures(commandManager, paths);
	}

	Material(size_t id, Device& device) : id(id), device(device) {}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Add a texture of the specified type
	void addTexture(TextureType type, ImageObjects texture);

	// Create all textures with a defined path in the struct texturePaths
	void createTextures(const CommandManager& commandManager, TexturePaths texturePaths);

	// Destroy Vulkan and other objects
	void cleanup();

	// Erase the texture handlers and destroy the Vulkan objects associated
	void destroyTextures();

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	using TextureTypes = uint32_t;

	Device& device;

	const size_t id;

	size_t textureCount = 0;
	TextureTypes usedTypes = TEXTURE_TYPE_NONE_BIT;

	ImageObjects albedoTexture;
	ImageObjects specularTexture;
	ImageObjects normalTexture;
	std::vector<ImageObjects> customTextures;

	uint32_t mipLevels = 1; // computed in createTexture() since them depend on the texture size
	// TODO: review other owner for the sampler
	VkSampler sampler = VK_NULL_HANDLE;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Create all the resources involved in texture usage and add it to the texture list
	void createTexture(const CommandManager& commandManager, TextureType type, std::string texturePath);
	
	// Create sampler for the textures
	void createSampler(uint32_t mipLevels);

	void storeTexture(TextureType type, ImageObjects texture);
};
