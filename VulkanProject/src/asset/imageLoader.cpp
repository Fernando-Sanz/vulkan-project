#include "ImageLoader.hpp"

#include <iostream>
#include <stdexcept>


RawImage loadImageFromFile(const std::string& path) {
    RawImage image;

    image.pixels = stbi_load(path.c_str(), &image.width, &image.height, nullptr, STBI_rgb_alpha);
    if (!image.pixels) throw std::runtime_error("failed to load texture image");

    image.size = image.width * image.height * 4/*bytes per pixel*/;

    std::cout << "Loaded texture ";
    std::cout << image.width << "x" << image.height << std::endl;

    return image;
}