#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include "Device.hpp"


class  Pipeline {

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

protected:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// INTERNAL STRUCT

	struct Attachment {
		VkAttachmentDescription description;
		VkAttachmentReference reference;
		bool isValid = true;
	};

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
	virtual void createRenderPass(VkFormat imageFormat, VkFormat depthFormat);

	virtual Attachment getColorAttachment(uint32_t id, VkFormat format);
	
	virtual Attachment getDepthAttachment(uint32_t id, VkFormat format);

	virtual Attachment getColorResolveAttachment(uint32_t id, VkFormat format);

	// Defines the Descriptor Set Layout of the pipeline
	virtual void createDescriptorSetLayout();

	// Create a Pipeline with all the stages and a PipelineLayout
	virtual void createGraphicsPipeline(std::string vertexShaderLocation, std::string fragmentShaderLocation);

	// Create a ShaderModule given its code
	VkShaderModule createShaderModule(const std::string& filename);


};
