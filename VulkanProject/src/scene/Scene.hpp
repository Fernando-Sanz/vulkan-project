#pragma once

#include <vector>
#include "scene/Entity.hpp"
#include "scene/Camera.hpp"


struct Scene {

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// FIELDS

	// TODO: review if it is better to use a map with Entity id
	std::vector<Entity> entities;
	Camera* activeCamera = nullptr;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Add an entity to the scene
	template<typename T>
	Entity& addEntity(T&& entity) {
		// Use perfect forwarding
		entities.push_back(std::forward<T>(entity));
		return entities.back();
	};

	// Return a vector with the modules of the specified type in the scene
	template<typename T>
	std::vector<T*> getModulesOfType() {

		std::vector<T*> targetModules;
		// Get the modules from all the entities
		for (Entity* entity : entities) {
			std::vector<T*> modules = entity->getModulesOfType();
			targetModules.insert(targetModules.begin(), modules.begin(), modules.end());
		}

		return targetModules;
	}
};
