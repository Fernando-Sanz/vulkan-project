#include "Pipeline.hpp"
#include <vulkan/vulkan.h>

#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>

#include "Vertex.hpp"


namespace {
	// TODO: move to utils type file
	std::vector<char> readFile(const std::string& filename) {

		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file");
		}

		// take the size
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		std::cout << "Shader " << filename << " with size: " << fileSize << std::endl;

		// read
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}
}


void Pipeline::create(Device device, VkFormat imageFormat, VkFormat depthFormat,
	std::string vertShaderLocation, std::string fragShaderLocation) {

	this->device = device;

	createRenderPass(imageFormat, depthFormat);
	createDescriptorSetLayout();
	createGraphicsPipeline(vertShaderLocation, fragShaderLocation);
}


void Pipeline::createRenderPass(VkFormat imageFormat, VkFormat depthFormat) {

	//----------------------------------------------------
	// GET THE ATTACHMENTS

	Attachment colorAtt = getColorAttachment(0, imageFormat);
	VkAttachmentDescription colorAttachment = colorAtt.description;
	VkAttachmentReference colorAttachmentRef = colorAtt.reference;

	Attachment depthAtt = getDepthAttachment(1, depthFormat);
	VkAttachmentDescription depthAttachment = depthAtt.description;
	VkAttachmentReference depthAttachmentRef = depthAtt.reference;

	Attachment colorResolveAtt = getColorResolveAttachment(2, imageFormat);
	VkAttachmentDescription colorResolveAttachment = colorResolveAtt.description;
	VkAttachmentReference colorResolveAttachmentRef = colorResolveAtt.reference;

	//----------------------------------------------------
	// RENDER SUBPASS

	std::vector<VkAttachmentDescription> attachments;
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;

	if (colorAtt.isValid) {
		attachments.push_back(colorAttachment);
		subpass.pColorAttachments = &colorAttachmentRef;
	}
	if (depthAtt.isValid) {
		attachments.push_back(depthAttachment);
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
	}
	if (colorResolveAtt.isValid) {
		attachments.push_back(colorResolveAttachment);
		subpass.pResolveAttachments = &colorResolveAttachmentRef;
	}

	//----------------------------------------------------
	// DEPENDENCIES
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Implicit first subpass
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	//----------------------------------------------------
	// CREATE RENDER PASS

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device.get(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass");
	}
}

Pipeline::Attachment Pipeline::getColorAttachment(uint32_t id, VkFormat format) {

	Attachment attachment{};

	attachment.description.format = format;
	attachment.description.samples = device.getMsaaSamples();
	attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachment.reference.attachment = id;
	attachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	return attachment;
}

Pipeline::Attachment Pipeline::getDepthAttachment(uint32_t id, VkFormat format) {

	Attachment attachment{};

	attachment.description.format = format;
	attachment.description.samples = device.getMsaaSamples();
	attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachment.reference.attachment = id;
	attachment.reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	return attachment;
}

Pipeline::Attachment Pipeline::getColorResolveAttachment(uint32_t id, VkFormat format) {
	
	Attachment attachment{};

	attachment.description.format = format;
	attachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachment.reference.attachment = id;
	attachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	return attachment;
}


void Pipeline::createDescriptorSetLayout() {

	//----------------------------------------------------
	// BINDINGS

	//---------------------
	// ubo binding
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	//---------------------
	// sampler binding
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	//----------------------------------------------------
	// CREATE DESCRIPTOR SET
	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	// CREATION
	if (vkCreateDescriptorSetLayout(device.get(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout");
	}
}


void Pipeline::createGraphicsPipeline(std::string vertShaderLocation, std::string fragShaderLocation) {

	//--------------------------------------------------------
	// SHADERS

	// create shader modules
	VkShaderModule vertShaderModule = createShaderModule(vertShaderLocation);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderLocation);

	// VERTEX SHADER
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	vertShaderStageInfo.pSpecializationInfo = nullptr; // For constants

	// FRAGMENT SHADER
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = nullptr; // For constants

	// SHADER STAGES
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };


	//--------------------------------------------------------
	// VERTEX INPUT

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


	//--------------------------------------------------------
	// INPUT ASSEMBLY

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;


	//--------------------------------------------------------
	// VIEWPORT AND SCISSORS (configured at drawing time)

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;


	//--------------------------------------------------------
	// RASTERIZER

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;

	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;

	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	// depth value changes
	rasterizer.depthBiasEnable = VK_FALSE;


	//--------------------------------------------------------
	// MULTISAMPLING

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_TRUE;
	multisampling.rasterizationSamples = device.getMsaaSamples();
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional


	//--------------------------------------------------------
	// DEPTH AND STENCIL

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional


	//--------------------------------------------------------
	// COLOR BLENDING (disabled)

	// per framebuffer
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	// global blending configuration
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;


	//--------------------------------------------------------
	// PIPELINE LAYOUT

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(device.get(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout");
	}


	//--------------------------------------------------------
	// DYNAMIC STATES

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();


	//--------------------------------------------------------
	//--------------------------------------------------------
	// CREATE GRAPHICS PIPELINE

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = pipelineLayout;

	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(device.get(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline");
	}


	//--------------------------------------------------------
	// DESTROY SHADER MODULES
	// They are not necessary, compilation to machine code already happens
	// in pipeline creation
	vkDestroyShaderModule(device.get(), fragShaderModule, nullptr);
	vkDestroyShaderModule(device.get(), vertShaderModule, nullptr);
}

//VkShaderModule Pipeline::createShaderModule(const std::vector<char>& code) {
VkShaderModule Pipeline::createShaderModule(const std::string& filename) {

	auto code = readFile(filename);

	// CREATE INFO
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size(); // Size in bytes (char)
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); // Vulkan uses uint32 pointer (not char)
	// It is not necessary to check alignment, std::vector already does

	// CREATE SHADER MODULE
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device.get(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module");
	}

	return shaderModule;
}


void Pipeline::cleanup() {
	vkDestroyDescriptorSetLayout(device.get(), descriptorSetLayout, nullptr);
	vkDestroyPipeline(device.get(), pipeline, nullptr);
	vkDestroyPipelineLayout(device.get(), pipelineLayout, nullptr);
	vkDestroyRenderPass(device.get(), renderPass, nullptr);
}
