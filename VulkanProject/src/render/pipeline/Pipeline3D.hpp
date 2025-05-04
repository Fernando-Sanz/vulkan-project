#pragma once

#include <vulkan/vulkan.h>

#include "render/pipeline/GraphicsPipeline.hpp"


class Pipeline3D : public GraphicsPipeline {
public:
	Pipeline3D() : GraphicsPipeline(true, true) {}

private:
	void createGraphicsPipeline(std::string vertexShaderLocation, std::string fragmentShaderLocation) override;
};
