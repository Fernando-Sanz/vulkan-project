#pragma once

#include <vector>
#include "scene/Entity.hpp"
#include "scene/Camera.hpp"


class Scene {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// FIELDS

	// TODO: review if it is better to use a map with Entity id
	std::vector<std::unique_ptr<Entity>> entities;
	Camera* activeCamera = nullptr;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Add an entity to the scene
	Entity* addEntity() {
		entities.push_back(std::make_unique<Entity>());
		return entities.back().get();
	};

	// Return a vector with the modules of the specified type in the scene
	// TODO: search in the children
	template<typename T>
	std::vector<T*> getModulesOfType() {

		std::vector<T*> targetModules;
		// Get the modules from all the entities
		for (auto& entity : entities) {
			std::vector<T*> modules = entity->getModulesOfType<T>();
			targetModules.insert(targetModules.end(), modules.begin(), modules.end());
		}

		return targetModules;
	}
};
