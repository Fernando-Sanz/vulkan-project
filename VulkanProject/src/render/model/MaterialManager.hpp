#pragma once 

#include <unordered_map>
#include <memory>
#include <string>

#include "context/Device.hpp"
#include "context/CommandManager.hpp"
#include "render/model/Material.hpp"


class MaterialManager {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	Material* getMaterial(size_t id) { return materials.count(id) > 0 ? materials[id].get() : nullptr; }
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS
	
	// Store the device and command manager for future operations
	void init(Device device, CommandManager commandManager);

	Material* createEmptyMaterial();

	// Create a material with all textures in the defined paths
	Material* createMaterial(TexturePaths texturePaths);

	void destroyMaterial(size_t id);

	// Destroy Vulkan and other objects
	void cleanup();

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	Device device;
	CommandManager commandManager;

	std::unordered_map<size_t, std::unique_ptr<Material>> materials;
	size_t nextId = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	Material* createMaterial(TexturePaths* texturePaths);
};
