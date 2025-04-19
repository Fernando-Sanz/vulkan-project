#pragma once

#include <vulkan/vulkan.h>

#include "render/pipeline/GraphicsPIpeline.hpp"


class  FirstPassPipeline : public GraphicsPipeline {

public:

private:
	
	void createRenderPass(VkFormat imageFormat, VkFormat depthFormat) override;
	void createGraphicsPipeline(std::string vertexShaderLocation, std::string fragmentShaderLocation) override;
};
