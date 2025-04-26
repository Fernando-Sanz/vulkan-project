#include "render/model/MeshManager.hpp"


void MeshManager::init(Device device, CommandManager commandManager) {
	this->device = device;
	this->commandManager = commandManager;
}

Mesh* MeshManager::createMesh(std::string modelPath) {

	auto [iterator, inserted] = meshes.insert(
		{ nextId, std::make_unique<Mesh>(nextId, device, commandManager, modelPath) }
	);
	nextId++;

	return iterator->second.get();
}


void MeshManager::destroyMesh(size_t id) {
	if (!meshes.count(id)) return;

	meshes[id]->cleanup();
	meshes.erase(id);
}

void MeshManager::cleanup() {
	for (const auto& pair : meshes) {
		destroyMesh(pair.first);
	}
}
