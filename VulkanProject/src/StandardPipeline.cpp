#include "StandardPipeline.hpp"

#include <array>
#include <stdexcept>

#include "Vertex.hpp"


StandardPipeline::Attachment StandardPipeline::getColorAttachment(uint32_t id, VkFormat format) {

	Attachment attachment{};

	attachment.description.format = format;
	attachment.description.samples = device.getMsaaSamples();
	attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // store the output
	attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachment.reference.attachment = id;
	attachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	return attachment;
}

StandardPipeline::Attachment StandardPipeline::getDepthAttachment(uint32_t id, VkFormat format) {

	Attachment attachment{};

	attachment.description.format = format;
	attachment.description.samples = device.getMsaaSamples();
	attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // store the output
	attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachment.reference.attachment = id;
	attachment.reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	return attachment;
}

StandardPipeline::Attachment StandardPipeline::getColorResolveAttachment(uint32_t id, VkFormat format) {

	Attachment attachment{};

	attachment.description.format = format;
	attachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // input for next pipeline

	attachment.reference.attachment = id;
	attachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	return attachment;
}
