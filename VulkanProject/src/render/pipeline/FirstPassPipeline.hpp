#pragma once

#include <vulkan/vulkan.h>

#include "render/pipeline/GraphicsPipeline.hpp"


class FirstPassPipeline : public GraphicsPipeline {
public:
	FirstPassPipeline() : GraphicsPipeline(true, true) {}

private:
	void createGraphicsPipeline(std::string vertexShaderLocation, std::string fragmentShaderLocation) override;
};
