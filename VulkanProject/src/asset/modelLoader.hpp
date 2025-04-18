#pragma once

#include "scene/Model.hpp"


struct ModelData {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};


ModelData loadModelFromFile(std::string& modelPath);
