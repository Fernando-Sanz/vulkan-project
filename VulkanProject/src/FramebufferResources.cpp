#include "FramebufferResources.hpp"

#include <array>


void FramebufferResources::createEmpty(Device device, VkExtent2D extent, VkRenderPass renderPass,
	VkFormat colorFormat) {

	this->device = device;
	this->extent = extent;
	this->renderPass = renderPass;

	createColorResources(colorFormat);
	createDepthResources(VK_IMAGE_USAGE_SAMPLED_BIT);
	createResolveResources(colorFormat, VK_IMAGE_USAGE_SAMPLED_BIT);
	createFramebuffer();
}

void FramebufferResources::createFromSwapChain(Device device, SwapChain swapChain, size_t imageIndex, VkRenderPass renderPass) {

	this->device = device;
	this->extent = swapChain.getExtent();
	this->renderPass = renderPass;

	createColorResources(swapChain.getImageFormat());
	createDepthResources(0); // no special usage
	resolveImage.view = swapChain.getImageView(imageIndex);
	createFramebuffer();
}

void FramebufferResources::createColorResources(VkFormat colorFormat) {
	VkSampleCountFlagBits msaaSamples = device.getMsaaSamples();

	createImage(device, extent.width, extent.height, 1, msaaSamples,
		colorFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		colorImage.image, colorImage.memory);
	colorImage.view = createImageView(device, colorImage.image, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void FramebufferResources::createDepthResources(VkImageUsageFlags usage) {
	VkSampleCountFlagBits msaaSamples = device.getMsaaSamples();
	VkFormat depthFormat = findDepthFormat(device);

	// Let the graphics pipeline change the layout of the image (implicit)
	// from UNDEFINED to DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	createImage(device, extent.width, extent.height, 1, msaaSamples,
		depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthImage.image, depthImage.memory);
	depthImage.view = createImageView(device, depthImage.image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void FramebufferResources::createResolveResources(VkFormat resolveFormat, VkImageUsageFlags usage) {

	createImage(device, extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT,
		resolveFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		resolveImage.image, resolveImage.memory);
	resolveImage.view = createImageView(device, resolveImage.image, resolveFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void FramebufferResources::createFramebuffer() {

	// Create framebuffer
	std::array<VkImageView, 3> attachments = {
		colorImage.view,
		depthImage.view,
		resolveImage.view
	};

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(device.get(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create first pass framebuffer");
	}
}

void FramebufferResources::cleanup() {
	destroyImageObjects(device, colorImage);
	destroyImageObjects(device, depthImage);
	if(resolveImage.image != VK_NULL_HANDLE)
	destroyImageObjects(device, resolveImage);

	vkDestroyFramebuffer(device.get(), framebuffer, nullptr);
}
