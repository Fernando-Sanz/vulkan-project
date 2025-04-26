#include "render/model/MaterialManager.hpp"


void MaterialManager::init(Device device, CommandManager commandManager) {
	this->device = device;
	this->commandManager = commandManager;
}

Material* MaterialManager::createEmptyMaterial() {
	return createMaterial(nullptr);
}

Material* MaterialManager::createMaterial(TexturePaths texturePaths) {
	return createMaterial(&texturePaths);
}

Material* MaterialManager::createMaterial(TexturePaths* texturePaths) {

	std::unique_ptr<Material> material;

	if (texturePaths)
		material = std::make_unique<Material>(nextId, device, commandManager, *texturePaths);
	else
		material = std::make_unique<Material>(nextId, device);

	auto [iterator, inserted] = materials.insert({ nextId, std::move(material) });
	nextId++;

	return iterator->second.get();
}

void MaterialManager::destroyMaterial(size_t id) {
	if (!materials.count(id)) return;

	materials[id]->cleanup();
	materials.erase(id);
}

void MaterialManager::cleanup() {
	for (const auto& pair : materials) {
		pair.second->cleanup();
	}
	materials.clear();
}
