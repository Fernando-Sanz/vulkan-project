#pragma once

#include <vulkan/vulkan.h>

#include "scene/Model.hpp"
#include "render/uniform/TextureManager.hpp"
#include "render/uniform/ModelUboManager.hpp"
#include "render/uniform/LightUboManager.hpp"


class GraphicsPipeline {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	VkRenderPass getRenderPass() { return renderPass; }
	VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS
	
	// Create the graphics pipeline with the specified formats
	void create(Device device, VkFormat imageFormat, VkFormat depthFormat, Model model, uint32_t lightCount,
		std::string vertShaderLocation, std::string fragShaderLocation);

	// Allocate descriptor sets with the layout of the pipeline
	void allocateDescriptorSets(VkDescriptorPool pool, uint32_t count, VkDescriptorSet* descriptorSets);

	// Write a descriptor set with the corresponding data
	void updateDescriptorSet(ModelUboManager modelUniforms, LightUboManager lightsUniforms, TextureManager textures,
		VkDescriptorSet descriptorSet);

	// Record a command buffer with the necessary operations to use the pipeline
	void recordDrawing(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkExtent2D extent,
		Model model, VkDescriptorSet descriptorSet);

	// Destroy Vulkan and other objects
	void cleanup();

protected:

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
	virtual void createRenderPass(VkFormat imageFormat, VkFormat depthFormat) = 0;

	// Defines the Descriptor Set Layout of the pipeline
	void createDescriptorSetLayout(Model model, uint32_t lightCount);

	// Create a GraphicsPipeline with all the stages and a PipelineLayout
	virtual void createGraphicsPipeline(std::string vertexShaderLocation, std::string fragmentShaderLocation) = 0;

	static VkShaderModule createShaderModule(Device device, const std::vector<char>& code);

};
