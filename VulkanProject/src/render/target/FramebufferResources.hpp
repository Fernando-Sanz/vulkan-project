#pragma once

#include "render/image/imageUtils.hpp"
#include "render/target/SwapChain.hpp"


class FramebufferResources {
public:
	VkFramebuffer get() { return framebuffer; }
	ImageObjects getResolveImage() { return resolveImage; }

	// Create a framebuffer with attachments prepared for sampling
	void createEmpty(Device device, VkExtent2D extent, VkRenderPass renderPass,
		VkFormat colorFormat);

	// Create a framebuffer using an image view of the swap chain specified
	void createFromSwapChain(Device device, SwapChain swapChain, size_t imageIndex, VkRenderPass renderPass);

	void cleanup();

private:
	Device device;

	VkFramebuffer framebuffer;

	ImageObjects colorImage;
	ImageObjects depthImage;
	ImageObjects resolveImage;

	VkRenderPass renderPass;
	VkExtent2D extent;

	void createColorResources(VkFormat colorFormat);
	void createDepthResources(VkImageUsageFlags usage);
	// usage is to choose between SAMPLED or PRESENT
	void createResolveResources(VkFormat resolveFormat, VkImageUsageFlags usage);

	void createFramebuffer();
};
