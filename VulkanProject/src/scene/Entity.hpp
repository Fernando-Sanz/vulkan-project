#pragma once

#include <vector>
#include <memory>

#include "scene/Transform.hpp"


class Model;
class Light;
class Camera;


class Entity {
public:

	Transform transform;

	Entity* parent;
	std::vector<std::unique_ptr<Entity>> children;

	// Modules
	std::vector<std::unique_ptr<Model>> models;
	std::vector<std::unique_ptr<Light>> lights;
	std::vector<std::unique_ptr<Camera>> cameras;
	// TODO: add a list of Behaviour modules
	// std::vector<std::unique_ptr<Behaviour>> behaviours;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS
	
	// Add a module to the entity
	template<typename T>
	T* addModule() {
		if constexpr (std::is_same_v<T, Model>) {
			models.push_back(std::make_unique<Model>());
			auto* model = models.back().get();
			model->setOwner(this);
			return model;
		}
		else if constexpr (std::is_same_v<T, Light>) {
			lights.push_back(std::make_unique<Light>());
			auto* light = lights.back().get();
			light->setOwner(this);
			return light;
		}
		else if constexpr (std::is_same_v<T, Camera>) {
			cameras.push_back(std::make_unique<Camera>());
			auto* camera = cameras.back().get();
			camera->setOwner(this);
			return camera;
		}
	}

	template<typename T>
	std::vector<T*> getModulesOfType() {
		std::vector<T*> pointers;
		if constexpr (std::is_same_v<T, Model>) {
			for (auto& model : models)
				pointers.push_back(model.get());
		}
		else if constexpr (std::is_same_v<T, Light>) {
			for (auto& light : lights)
				pointers.push_back(light.get());
		}
		else if constexpr (std::is_same_v<T, Camera>) {
			for (auto& camera : cameras)
				pointers.push_back(camera.get());
		}
		return pointers;
	}
};

