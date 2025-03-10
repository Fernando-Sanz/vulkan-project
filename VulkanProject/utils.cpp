#include "utils.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Vertex.hpp"
#include <iostream>

unsigned char* loadImage(const char* texturePath, int* texWidth, int* texHeight, int* texChannels) {

	return stbi_load(texturePath, texWidth, texHeight, texChannels, STBI_rgb_alpha);
}

void freeImage(unsigned char* pixels) {
	stbi_image_free(pixels);
}

void loadAModel(std::string modelPath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {

	// Elements loaded from obj file
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	// LOAD THE MODEL
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
		throw std::runtime_error(warn + err);
	}


	// POPULLATE THE VERTICES

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			// position
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			// texture coordinates
			//	the vertical component is flipped for correct visualization (OBJ to Vulkan conversion)
			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			// base color
			vertex.color = { 1.0f, 1.0f, 1.0f };

			// check if the vertex already exists and store the index
			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}

	std::cout << "Model loaded: " << vertices.size() << " vertices" << std::endl;
}
