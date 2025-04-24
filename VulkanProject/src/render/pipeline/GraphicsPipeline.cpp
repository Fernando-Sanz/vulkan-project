#include "render/pipeline/GraphicsPIpeline.hpp"


namespace Bindings {
	void addModelBinding(std::vector<VkDescriptorSetLayoutBinding>& bindings) {
		VkDescriptorSetLayoutBinding modelUboLayoutBinding{};
		modelUboLayoutBinding.binding = static_cast<uint32_t>(bindings.size());
		modelUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		modelUboLayoutBinding.descriptorCount = 1; // only 1 model per render pass
		modelUboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		
		bindings.push_back(modelUboLayoutBinding);
	}

	void addTextureBindings(std::vector<VkDescriptorSetLayoutBinding>& bindings, int textureCount) {
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = static_cast<uint32_t>(bindings.size());
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr; // Optional

		bindings.push_back(samplerLayoutBinding);

		VkDescriptorSetLayoutBinding texturesLayoutBinding{};
		texturesLayoutBinding.binding = static_cast<uint32_t>(bindings.size());
		texturesLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		texturesLayoutBinding.descriptorCount = textureCount;
		texturesLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		bindings.push_back(texturesLayoutBinding);
	}

	void addLightBinding(std::vector<VkDescriptorSetLayoutBinding>& bindings, int lightCount) {
		VkDescriptorSetLayoutBinding lightUboLayoutBinding{};
		lightUboLayoutBinding.binding = static_cast<uint32_t>(bindings.size());
		lightUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightUboLayoutBinding.descriptorCount = lightCount;
		lightUboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		bindings.push_back(lightUboLayoutBinding);
	}
}

namespace DescriptorSets {
	void addModelDescriptorWrite(VkBuffer modelUboBuffer, VkDescriptorSet descriptorSetDst,
		VkDescriptorBufferInfo& modelBufferInfo, std::vector<VkWriteDescriptorSet>& descriptorWrites) {

		// INFO
		modelBufferInfo.buffer = modelUboBuffer;
		modelBufferInfo.offset = 0;
		modelBufferInfo.range = sizeof(ModelUBO);

		// WRITE
		VkWriteDescriptorSet modelBufferWrite{};
		modelBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		modelBufferWrite.dstSet = descriptorSetDst;
		modelBufferWrite.dstBinding = static_cast<uint32_t>(descriptorWrites.size());
		modelBufferWrite.dstArrayElement = 0;
		modelBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		modelBufferWrite.descriptorCount = 1;
		modelBufferWrite.pBufferInfo = &modelBufferInfo;
		modelBufferWrite.pImageInfo = nullptr; // Not used
		modelBufferWrite.pTexelBufferView = nullptr; // Not used

		descriptorWrites.push_back(modelBufferWrite);
	}

	void addTextureDescriptorWrites(const Material& material, VkDescriptorSet descriptorSetDst,
		VkDescriptorImageInfo& samplerInfo, std::vector<VkDescriptorImageInfo>& textureInfos,
		std::vector<VkWriteDescriptorSet>& descriptorWrites) {

		// TEXTURE SAMPLER INFO
		samplerInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		samplerInfo.sampler = material.sampler;

		// TEXTURE INFOS

		textureInfos.resize(material.textureCount);
		int index = 0;
		if (material.hasAlbedo()) {
			// color
			textureInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			textureInfos[index].imageView = material.albedoTexture.view;
			index++;
		}
		if (material.hasSpecular()) {
			// specular
			textureInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			textureInfos[index].imageView = material.specularTexture.view;
			index++;
		}
		if (material.hasNormal()) {
			// normal
			textureInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			textureInfos[index].imageView = material.normalTexture.view;
			index++;
		}
		if (material.customTextures.size()) {
			// custom
			for (size_t i = 0; i < material.customTextures.size(); i++) {
				textureInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				textureInfos[index].imageView = material.customTextures[i].view;
				index++;
			}
		}

		// WRITES

		// texture sampler
		VkWriteDescriptorSet samplerWrite{};
		samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		samplerWrite.dstSet = descriptorSetDst;
		samplerWrite.dstBinding = static_cast<uint32_t>(descriptorWrites.size());
		samplerWrite.dstArrayElement = 0;
		samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		samplerWrite.descriptorCount = 1;
		samplerWrite.pImageInfo = &samplerInfo;

		descriptorWrites.push_back(samplerWrite);

		// textures
		VkWriteDescriptorSet texturesWrite{};
		texturesWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		texturesWrite.dstSet = descriptorSetDst;
		texturesWrite.dstBinding = static_cast<uint32_t>(descriptorWrites.size());
		texturesWrite.dstArrayElement = 0;
		texturesWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		texturesWrite.descriptorCount = static_cast<uint32_t>(textureInfos.size());
		texturesWrite.pImageInfo = textureInfos.data();

		descriptorWrites.push_back(texturesWrite);
	}

	void addLightDescriptorWrite(VkBuffer lightsUboBuffer, size_t lightCount, VkDescriptorSet descriptorSetDst,
		VkDescriptorBufferInfo& lightsBufferInfo, std::vector<VkWriteDescriptorSet>& descriptorWrites) {

		// INFO
		lightsBufferInfo.buffer = lightsUboBuffer;
		lightsBufferInfo.offset = 0;
		lightsBufferInfo.range = sizeof(LightUBO) * lightCount;

		// WRITE
		VkWriteDescriptorSet lightsBufferWrite{};
		lightsBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		lightsBufferWrite.dstSet = descriptorSetDst;
		lightsBufferWrite.dstBinding = static_cast<uint32_t>(descriptorWrites.size());
		lightsBufferWrite.dstArrayElement = 0;
		lightsBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lightsBufferWrite.descriptorCount = 1;
		lightsBufferWrite.pBufferInfo = &lightsBufferInfo;

		descriptorWrites.push_back(lightsBufferWrite);
	}
}

void GraphicsPipeline::create(Device device, VkFormat imageFormat, VkFormat depthFormat, Model* model, uint32_t lightCount,
	std::string vertShaderLocation, std::string fragShaderLocation) {

	this->device = device;

	createRenderPass(imageFormat, depthFormat);
	createDescriptorSetLayout(model, lightCount);
	createGraphicsPipeline(vertShaderLocation, fragShaderLocation);
}


void GraphicsPipeline::createDescriptorSetLayout(Model* model, uint32_t lightCount) {

	//----------------------------------------------------
	// BINDINGS
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	// MODEL UBO
	if (!model->useRawVertexData())
		Bindings::addModelBinding(bindings);

	// SAMPLER AND TEXTURE
	size_t textureCount = model->getMaterial().textureCount;
	if (textureCount > 0) {
		Bindings::addTextureBindings(bindings, textureCount);
	}

	// LIGHT UBOS
	if (lightCount > 0) {
		Bindings::addLightBinding(bindings, lightCount);
	}

	//----------------------------------------------------
	// CREATE DESCRIPTOR SET LAYOUT
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
	Model* model, VkDescriptorSet descriptorSet) {

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
	VkBuffer vertexBuffers[] = { model->getVertexBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	// index buffer
	vkCmdBindIndexBuffer(commandBuffer, model->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

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
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model->getIndices().size()), 1, 0, 0, 0);
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
void GraphicsPipeline::updateDescriptorSet(Material material, VkDescriptorSet descriptorSet) {
	updateDescriptorSet({}, material, {}, descriptorSet);
}

// TODO: move this to a Renderer class
void GraphicsPipeline::updateDescriptorSet(ModelUboManager modelUniforms, Material material, LightUboManager lightsUniforms,
	VkDescriptorSet descriptorSet) {

	// DESCRIPTOR WRITES
	std::vector<VkWriteDescriptorSet> descriptorWrites{};

	// Model UBO
	VkDescriptorBufferInfo modelBufferInfo;
	if (modelUniforms.hasModel()) {
		DescriptorSets::addModelDescriptorWrite(modelUniforms.getBuffer(0), descriptorSet, modelBufferInfo, descriptorWrites);
	}

	// Sample and textures
	VkDescriptorImageInfo samplerInfo{};
	std::vector<VkDescriptorImageInfo> textureInfos;
	if (material.textureCount > 0) {
		DescriptorSets::addTextureDescriptorWrites(material, descriptorSet, samplerInfo, textureInfos, descriptorWrites);
	}

	// Lights UBO
	VkDescriptorBufferInfo lightsBufferInfo{};
	if (lightsUniforms.getLightCount() > 0) {
		DescriptorSets::addLightDescriptorWrite(lightsUniforms.getBuffer(0), lightsUniforms.getLightCount(), descriptorSet,
			lightsBufferInfo, descriptorWrites);
	}
	
	// UPDATE
	vkUpdateDescriptorSets(device.get(), static_cast<uint32_t>(descriptorWrites.size()),
		descriptorWrites.data(), 0, nullptr);
}

VkShaderModule GraphicsPipeline::createShaderModule(Device device, const std::vector<char>& code) {
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

void GraphicsPipeline::cleanup() {
	vkDestroyDescriptorSetLayout(device.get(), descriptorSetLayout, nullptr);
	vkDestroyPipeline(device.get(), pipeline, nullptr);
	vkDestroyPipelineLayout(device.get(), pipelineLayout, nullptr);
	vkDestroyRenderPass(device.get(), renderPass, nullptr);
}
