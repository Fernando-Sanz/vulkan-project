#include "modelLoader.hpp"

#include <iostream>
#include <tiny_obj_loader.h>


ModelData loadModelFromFile(std::string& modelPath) {

	ModelData modelData;
	//--------------------------------------------------------
	// Load obj file

	tinyobj::ObjReader reader;
	tinyobj::ObjReaderConfig reader_config;

	if (!reader.ParseFromFile(modelPath, reader_config)) {
		if (!reader.Error().empty()) {
			throw std::runtime_error(reader.Error());
		}
	}

	if (!reader.Warning().empty()) {
		std::cout << "TinyObjLoader: " << reader.Warning();
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();

	//--------------------------------------------------------
	// Fill in the model with the loaded data

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	// SHAPES LOOP
	for (const auto& shape : shapes) {
		// FACES LOOP
		for (const auto& index : shape.mesh.indices) {

			Vertex vertex{};

			//-------------------------
			// POSITION
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			//-------------------------
			// NORMAL
			if (index.normal_index >= 0) {
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
			}

			//-------------------------
			// TEXTURE COORDINATES
			if (index.texcoord_index >= 0) {
				// flip vertical component for correct visualization (OBJ to Vulkan conversion)
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}

			//-------------------------
			// check if the vertex already exists and store the index
			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(modelData.vertices.size());
				modelData.vertices.push_back(vertex);
			}

			modelData.indices.push_back(uniqueVertices[vertex]);
		}
	}

	std::cout << "Model loaded: " << modelData.vertices.size() << " vertices" << std::endl;
	return modelData;
}
