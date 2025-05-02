#pragma once

#include <vulkan/vulkan.h>

#include "context/Device.hpp"


class RenderPasses {
public:

	static const VkRenderPass& getFirstRenderPass() { return firstRenderPass; }
	static const VkRenderPass& getSecondRenderPass() { return secondRenderPass; }

	static void createRenderPasses(Device* device, VkFormat imageFormat, VkFormat depthFormat);

	static void cleanup();

private:

	static Device* device;

	static VkRenderPass firstRenderPass;
	static VkRenderPass secondRenderPass;

	static void createFirstRenderPass(VkFormat imageFormat, VkFormat depthFormat);
	static void createSecondRenderPass(VkFormat imageFormat, VkFormat depthFormat);

};
