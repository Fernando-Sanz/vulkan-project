#pragma once

#include <string>
#include <stb_image.h>


struct RawImage {
    stbi_uc* pixels = nullptr;

    int width = 0;
    int height = 0;

    int size = 0;

    void free() { stbi_image_free(pixels); pixels = nullptr; }
};


RawImage loadImageFromFile(const std::string& path);
