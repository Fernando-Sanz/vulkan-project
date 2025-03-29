#pragma once

#include <vulkan/vulkan.h>

#include "Pipeline.hpp"


class StandardPipeline : public Pipeline {
public:

private:

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// METHODS

	Attachment getColorAttachment(uint32_t id, VkFormat format) override;

};
