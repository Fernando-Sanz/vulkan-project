#include "asset/bytecodeFileReader.hpp"

#include <stdexcept>
#include <iostream>
#include <fstream>


std::vector<char> readFile(const std::string& filename) {

	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file");
	}

	// take the size
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	std::cout << "Shader " << filename << " with size: " << fileSize << std::endl;

	// read
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}
