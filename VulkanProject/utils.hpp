#pragma once
#include <string>
#include "Vertex.hpp"

unsigned char* loadImage(const char* texturePath, int* texWidth, int* texHeight, int* texChannels);

void freeImage(unsigned char* pixels);

void loadAModel(std::string modelPath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
