#pragma once

#include <vulkan/vulkan.h>

#include "GraphicsPipeline.hpp"


class  SecondPassPipeline : public GraphicsPipeline{

public:

private:
	
	void createRenderPass(VkFormat imageFormat, VkFormat depthFormat) override;
	void createDescriptorSetLayout(uint32_t textureCount) override;
	void createGraphicsPipeline(std::string vertexShaderLocation, std::string fragmentShaderLocation) override;

};
