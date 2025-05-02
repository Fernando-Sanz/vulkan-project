#pragma once

#include <vulkan/vulkan.h>

#include "render/pipeline/GraphicsPipeline.hpp"


class Pipeline2D : public GraphicsPipeline {
public:
	Pipeline2D() : GraphicsPipeline(false, false) {}

private:
	void createGraphicsPipeline(std::string vertexShaderLocation, std::string fragmentShaderLocation) override;

};
