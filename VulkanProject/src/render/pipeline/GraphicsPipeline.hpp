#pragma once

#include <vulkan/vulkan.h>

#include "scene/Model.hpp"
#include "render/model/Material.hpp"
#include "render/uniform/ModelUboManager.hpp"
#include "render/uniform/LightUboManager.hpp"


class GraphicsPipeline {
public:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTERS AND SETTERS

	const VkRenderPass& getRenderPass() const { return renderPass; }
	VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; }

	GraphicsPipeline(bool useModelUBO, bool useLightUBO) : useModelUBO(useModelUBO), useLightUBO(useLightUBO) {}
	virtual ~GraphicsPipeline() = default;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS
	
	// Create the graphics pipeline with the specified formats
	void create(Device device, VkRenderPass renderPass, uint32_t textureCount,
		std::string vertShaderLocation, std::string fragShaderLocation);

	// Record a command buffer with the necessary operations to use the pipeline
	void recordDrawing(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkExtent2D extent,
		Model* model, VkDescriptorSet descriptorSet);

	// Destroy Vulkan and other objects
	void cleanup();

	// TODO: MOVE TO RENDERER OR SIMILAR
	// Allocate descriptor sets with the layout of the pipeline
	void allocateDescriptorSets(VkDescriptorPool pool, uint32_t count, VkDescriptorSet* descriptorSets);
	// Write a descriptor set with the corresponding data
	void updateDescriptorSet(ModelUboManager modelUniforms, const Material* material, LightUboManager lightsUniforms,
		VkDescriptorSet descriptorSet);
	// Write a descriptor set with the corresponding data
	void updateDescriptorSet(const Material* material, VkDescriptorSet descriptorSet);

protected:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CLASS MEMBERS

	Device device;

	VkPipeline pipeline;
	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;

	const bool useModelUBO;
	const bool useLightUBO;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	// Defines the Descriptor Set Layout of the pipeline
	void createDescriptorSetLayout(uint32_t textureCount);

	// Create a GraphicsPipeline with all the stages and a PipelineLayout
	virtual void createGraphicsPipeline(std::string vertexShaderLocation, std::string fragmentShaderLocation) = 0;

	static VkShaderModule createShaderModule(Device device, const std::vector<char>& code);

};
