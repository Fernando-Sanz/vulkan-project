#pragma once

#include <vulkan/vulkan.h>

#include "render/pipeline/GraphicsPipeline.hpp"


class SecondPassPipeline : public GraphicsPipeline {
public:
	SecondPassPipeline() : GraphicsPipeline(false, false) {}

private:
	void createGraphicsPipeline(std::string vertexShaderLocation, std::string fragmentShaderLocation) override;

};
