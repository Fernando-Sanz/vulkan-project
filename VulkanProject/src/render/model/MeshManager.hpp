#pragma once

#include <unordered_map>
#include <memory>
#include <string>

#include "context/Device.hpp"
#include "context/CommandManager.hpp"
#include "Mesh.hpp"


class MeshManager {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	Mesh* getMesh(size_t id) { return meshes.count(id) > 0 ? meshes[id].get() : nullptr; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	void init(Device device, CommandManager commandManager);

	// Create and fill a Mesh struct with the data of the file
	Mesh* createMesh(std::string modelPath);

	void destroyMesh(size_t id);

	void cleanup();

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	Device device;
	CommandManager commandManager;

	std::unordered_map<size_t, std::unique_ptr<Mesh>> meshes;
	size_t nextId = 0;

};
