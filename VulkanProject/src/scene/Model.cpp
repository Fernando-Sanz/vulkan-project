#include "Model.hpp"

#include "asset/modelLoader.hpp"


void Model::create(Mesh* mesh, Material* material, bool useRawVertexData) {

	this->mesh = mesh;
	this->material = material;

	//--------------------------------------------------------
	// TRANSFORM (if the vertex data will be used with transformations)
	if (!useRawVertexData) {
		transform = &entity->transform;
	}
}
