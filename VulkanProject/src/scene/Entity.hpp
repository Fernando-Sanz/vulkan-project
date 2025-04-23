#pragma once

#include <vector>

#include "scene/Transform.hpp"
//#include "scene/Model.hpp"
//#include "scene/Light.hpp"
//#include "scene/Camera.hpp"
class Model;
class Light;
class Camera;


class Entity {
public:

	Transform transform;

	Entity* parent;
	std::vector<Entity> children;

	// Modules
	std::vector<Model> models;
	std::vector<Light> lights;
	std::vector<Camera> cameras;
	// TODO: add a list of Behaviour modules
	// std::vector<Behaviour> behaviours;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS
	
	// Add a module to the entity
	template<typename T>
	T& addModule(const T& module) {
		if constexpr (std::is_same_v<T, Model>) {
			models.push_back(module);
			models.back().setOwner(this);
			return models.back();
		}
		else if constexpr (std::is_same_v<T, Light>) {
			lights.push_back(module);
			lights.back().setOwner(this);
			return lights.back();
		}
		else if constexpr (std::is_same_v<T, Camera>) {
			cameras.push_back(module);
			cameras.back().setOwner(this);
			return cameras.back();
		}
	}

	template<typename T>
	std::vector<T>& getModulesOfType() {
		if constexpr (std::is_same_v<T, Model>) {
			return models;
		}
		else if constexpr (std::is_same_v<T, Light>) {
			return lights;
		}
		else if constexpr (std::is_same_v<T, Camera>) {
			return cameras;
		}
	}
};

