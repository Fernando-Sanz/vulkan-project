#pragma once

#include <vector>

#include "scene/Transform.hpp"
#include "scene/Module.hpp"


class Module;


struct Entity {

	Transform transform;

	Entity* parent;
	std::vector<Entity> children;
	std::vector<Module> modules;
	// TODO: add a list of Behaviour modules
	// std::vector<Behaviour> behaviours;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS
	
	// Add a module to the entity
	template <typename T>
	Module& addModule(T&& module) {
		// Use perfect forwarding
		modules.push_back(std::forward<T>(module));
		Module& m = modules.back();
		m.setOwner(this);
		return m;
	}

	// Return a vector with the modules of the specified type in the entity
	template<typename T>
	std::vector<T*> getModulesOfType() {

		std::vector<T*> targetModules;
		for (Module* module : modules) {
			if (T* targetModule = dynamic_cast<T*>(module))
				targetModules.push_back(targetModule);
		}

		return targetModules;
	}

};

