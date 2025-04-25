#pragma once

#include "scene/Module.hpp"
#include "render/vertex/Mesh.hpp"
#include "render/uniform/Material.hpp"


class Model : public Module {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	bool useRawVertexData() { return transform == nullptr; }

	Mesh* getMesh() { return mesh; }
	Material* getMaterial() { return material; }

	void setMaterial(Material* material) { this->material = material; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Load a model, create its vertex and index buffer and its textures. If useRawVertexData is true then the transform
	// is not initialized (the shader will use the raw vertex data without transformations)
	void create(Mesh* mesh, Material* material, bool useRawVertexData = false);

	void setOwner(Entity* owner) override {
		Module::setOwner(owner);
		transform = nullptr;
	}

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	Mesh* mesh;
	Material* material;
};
