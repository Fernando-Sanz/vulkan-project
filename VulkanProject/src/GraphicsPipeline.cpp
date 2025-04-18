#include "GraphicsPipeline.hpp"


// TODO: receive a vector of Model and iterate over them to get their textures
void GraphicsPipeline::create(Device device, VkFormat imageFormat, VkFormat depthFormat,
	bool renderModel, uint32_t textureCount, uint32_t lightCount,
	std::string vertShaderLocation, std::string fragShaderLocation) {

	this->device = device;

	createRenderPass(imageFormat, depthFormat);
	createDescriptorSetLayout(
		renderModel, textureCount, lightCount);
	createGraphicsPipeline(vertShaderLocation, fragShaderLocation);
}


void GraphicsPipeline::createDescriptorSetLayout(bool renderModel, uint32_t textureCount,
	uint32_t lightCount) {

	//----------------------------------------------------
	// BINDINGS

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	//---------------------
	// model UBO binding
	VkDescriptorSetLayoutBinding modelUboLayoutBinding{};
	if (renderModel) {
		modelUboLayoutBinding.binding = static_cast<uint32_t>(bindings.size());
		modelUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		modelUboLayoutBinding.descriptorCount = 1; // only 1 model per render pass
		modelUboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		
		bindings.push_back(modelUboLayoutBinding);
	}

	//---------------------
	// sampler and textures bindings
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	VkDescriptorSetLayoutBinding texturesLayoutBinding{};
	if (textureCount > 0) {
		samplerLayoutBinding.binding = static_cast<uint32_t>(bindings.size());
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr; // Optional

		bindings.push_back(samplerLayoutBinding);

		texturesLayoutBinding.binding = static_cast<uint32_t>(bindings.size());
		texturesLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		texturesLayoutBinding.descriptorCount = textureCount;
		texturesLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		bindings.push_back(texturesLayoutBinding);
	}

	//---------------------
	// ligth UBO binding
	VkDescriptorSetLayoutBinding lightUboLayoutBinding{};
	if (lightCount > 0) {
		lightUboLayoutBinding.binding = static_cast<uint32_t>(bindings.size());
		lightUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightUboLayoutBinding.descriptorCount = lightCount;
		lightUboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		bindings.push_back(lightUboLayoutBinding);
	}

	//----------------------------------------------------
	// CREATE DESCRIPTOR SET

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	// CREATION
	if (vkCreateDescriptorSetLayout(device.get(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout");
	}
}

void GraphicsPipeline::recordDrawing(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkExtent2D extent,
	Model model, VkDescriptorSet descriptorSet) {

	//---------------------
	// RENDER PASS
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = framebuffer;

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = extent;

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };	// for color attachment
	clearValues[1].depthStencil = { 1.0f, 0 };				// for depth attachment

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	// drawing commands
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	//---------------------
	// PIPELINE DATA

	// vertex buffers
	VkBuffer vertexBuffers[] = { model.getVertexBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	// index buffer
	vkCmdBindIndexBuffer(commandBuffer, model.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

	// viewport and scissor stage
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	//---------------------
	// DRAW GEOMETRY AND END
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.getIndices().size()), 1, 0, 0, 0);
	vkCmdEndRenderPass(commandBuffer);
}

// TODO: store inside a 'DescriptorPool' class all allocation requests and do all of them with one call to Vulkan
void GraphicsPipeline::allocateDescriptorSets(VkDescriptorPool pool, uint32_t count, VkDescriptorSet* descriptorSets) {
	std::vector<VkDescriptorSetLayout> layouts(count, descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = count;
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(device.get(), &allocInfo, descriptorSets) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate first pass descriptor set");
	}
}

// TODO: move this to a Renderer class
void GraphicsPipeline::updateDescriptorSet(ModelUboManager modelUniforms, LightUboManager lightsUniforms, TextureManager textures,
	VkDescriptorSet& descriptorSet) {

	// DESCRIPTOR WRITES
	std::vector<VkWriteDescriptorSet> descriptorWrites{};
	int binding = 0;

	// MODELS BUFFER
	VkDescriptorBufferInfo modelBufferInfo{};
	VkWriteDescriptorSet modelBufferWrite{};
	if (modelUniforms.hasModel()) {

		// INFO
		modelBufferInfo.buffer = modelUniforms.getBuffer(0); // TODO: RECEIVE AN ID WITH THE CORRECT MODEL
		modelBufferInfo.offset = 0;
		modelBufferInfo.range = sizeof(ModelUBO);

		// WRITE
		modelBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		modelBufferWrite.dstSet = descriptorSet;
		modelBufferWrite.dstBinding = binding++;
		modelBufferWrite.dstArrayElement = 0;
		modelBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		modelBufferWrite.descriptorCount = 1;
		modelBufferWrite.pBufferInfo = &modelBufferInfo;
		modelBufferWrite.pImageInfo = nullptr; // Not used
		modelBufferWrite.pTexelBufferView = nullptr; // Not used

		descriptorWrites.push_back(modelBufferWrite);
	}

	// SAMPLER AND TEXTURES
	VkDescriptorImageInfo samplerInfo{};
	std::vector<VkDescriptorImageInfo> textureInfos;
	VkWriteDescriptorSet samplerWrite{};
	VkWriteDescriptorSet texturesWrite{};
	if (textures.getTextureCount() > 0) {

		// TEXTURE SAMPLER INFO
		samplerInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		samplerInfo.sampler = textures.getSampler();

		// TEXTURE INFOS

		textureInfos.resize(textures.getTextureCount());
		auto usedTypes = textures.getTextureTypesUsed();
		int index = 0;
		if (usedTypes & TEXTURE_TYPE_ALBEDO_BIT) {
			// color
			textureInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			textureInfos[index].imageView = textures.getAlbedo().view;
			index++;
		}
		if (usedTypes & TEXTURE_TYPE_SPECULAR_BIT) {
			// specular
			textureInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			textureInfos[index].imageView = textures.getSpecular().view;
			index++;
		}
		if (usedTypes & TEXTURE_TYPE_NORMAL_BIT) {
			// normal
			textureInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			textureInfos[index].imageView = textures.getNormal().view;
			index++;
		}
		if (usedTypes & TEXTURE_TYPE_CUSTOM_BIT) {
			// custom
			for (auto& texture : textures.getCustomTextures()) {
				textureInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				textureInfos[index].imageView = texture.view;
				index++;
			}
		}

		// WRITES

		// texture sampler
		samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		samplerWrite.dstSet = descriptorSet;
		samplerWrite.dstBinding = binding++;
		samplerWrite.dstArrayElement = 0;
		samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		samplerWrite.descriptorCount = 1;
		samplerWrite.pImageInfo = &samplerInfo;

		descriptorWrites.push_back(samplerWrite);

		// textures
		texturesWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		texturesWrite.dstSet = descriptorSet;
		texturesWrite.dstBinding = binding++;
		texturesWrite.dstArrayElement = 0;
		texturesWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		texturesWrite.descriptorCount = static_cast<uint32_t>(textureInfos.size());
		texturesWrite.pImageInfo = textureInfos.data();

		descriptorWrites.push_back(texturesWrite);
	}

	// LIGHTS BUFFER
	VkDescriptorBufferInfo lightsBufferInfo{};
	VkWriteDescriptorSet lightsBufferWrite{};
	if (lightsUniforms.getLightCount() > 0) {

		// INFO
		lightsBufferInfo.buffer = lightsUniforms.getBuffer(0);
		lightsBufferInfo.offset = 0;
		lightsBufferInfo.range = sizeof(LightUBO) * lightsUniforms.getLightCount();

		// WRITE
		lightsBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		lightsBufferWrite.dstSet = descriptorSet;
		lightsBufferWrite.dstBinding = binding++;
		lightsBufferWrite.dstArrayElement = 0;
		lightsBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightsBufferWrite.descriptorCount = 1;
		lightsBufferWrite.pBufferInfo = &lightsBufferInfo;

		descriptorWrites.push_back(lightsBufferWrite);
	}
	
	// UPDATE
	vkUpdateDescriptorSets(device.get(), static_cast<uint32_t>(descriptorWrites.size()),
		descriptorWrites.data(), 0, nullptr);
}


void GraphicsPipeline::cleanup() {
	vkDestroyDescriptorSetLayout(device.get(), descriptorSetLayout, nullptr);
	vkDestroyPipeline(device.get(), pipeline, nullptr);
	vkDestroyPipelineLayout(device.get(), pipelineLayout, nullptr);
	vkDestroyRenderPass(device.get(), renderPass, nullptr);
}
