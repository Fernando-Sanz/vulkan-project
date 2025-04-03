#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include "Device.hpp"


class  StandardPipeline {

public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	VkPipeline get() { return pipeline; }
	VkRenderPass getRenderPass() { return renderPass; }
	VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; }
	VkPipelineLayout getLayout() { return pipelineLayout; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Create the graphics pipeline with the specified formats
	void create(Device device, VkFormat imageFormat, VkFormat depthFormat,
		std::string vertexShaderLocation, std::string fragmentShaderLocation);

	// Destroy Vulkan and other objects
	void cleanup();

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	Device device;

	VkPipeline pipeline;
	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Create RenderPass defining attachments, subpasses and dependencies
	void createRenderPass(VkFormat imageFormat, VkFormat depthFormat);

	// Defines the Descriptor Set Layout of the pipeline
	void createDescriptorSetLayout();

	// Create a GraphicsPipeline with all the stages and a PipelineLayout
	void createGraphicsPipeline(std::string vertexShaderLocation, std::string fragmentShaderLocation);

};
