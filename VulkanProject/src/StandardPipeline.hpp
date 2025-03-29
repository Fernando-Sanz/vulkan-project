#pragma once

#include <vulkan/vulkan.h>

#include "Pipeline.hpp"


// TODO: review the name, store the color is not standard
class StandardPipeline : public Pipeline {
public:

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	
	Attachment getColorAttachment(uint32_t id, VkFormat format) override;

	Attachment getDepthAttachment(uint32_t id, VkFormat format) override;

	Attachment getColorResolveAttachment(uint32_t id, VkFormat format) override;

};
