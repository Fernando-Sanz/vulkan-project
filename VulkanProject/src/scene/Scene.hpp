#pragma once

#include <vector>
#include "scene/Entity.hpp"
#include "scene/Camera.hpp"


class Scene {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// FIELDS

	// TODO: review if it is better to use a map with Entity id
	std::vector<Entity> entities;
	Camera* activeCamera = nullptr;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Add an entity to the scene
	Entity& addEntity() {
		entities.emplace_back();
		return entities.back();
	};

	// Return a vector with the modules of the specified type in the scene
	template<typename T>
	std::vector<T> getModulesOfType() {

		std::vector<T> targetModules;
		// Get the modules from all the entities
		for (Entity& entity : entities) {
			std::vector<T>& modules = entity.getModulesOfType<T>();
			targetModules.insert(targetModules.end(), modules.begin(), modules.end());
		}

		return targetModules;
	}
};
