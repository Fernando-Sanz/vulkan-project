#pragma once

#include <vulkan/vulkan.h>

#include "Model.hpp"
#include "TextureManager.hpp"
#include "UniformManager.hpp"


class GraphicsPipeline {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	VkRenderPass getRenderPass() { return renderPass; }
	VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS
	
	// Create the graphics pipeline with the specified formats
	void create(Device device, VkFormat imageFormat, VkFormat depthFormat,
		uint32_t modelCount, uint32_t lightCount, TextureManager textures,
		std::string vertShaderLocation, std::string fragShaderLocation);

	void allocateDescriptorSets(VkDescriptorPool pool, VkDescriptorSet& descriptorSet);

	void updateDescriptorSets(std::optional<UniformManager> uniformManager, std::optional<TextureManager> textures,
		VkDescriptorSet& descriptorSet);

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
	void createDescriptorSetLayout(uint32_t modelCount, uint32_t lightCount, uint32_t textureCount);

	// Create a GraphicsPipeline with all the stages and a PipelineLayout
	virtual void createGraphicsPipeline(std::string vertexShaderLocation, std::string fragmentShaderLocation) = 0;

};
