#include "vkUtil.h"

vk::ShaderModule CreateShaderModule(const std::string& path, vk::Device device) {
	std::ifstream loadFile(path, std::ios::ate | std::ios::binary);
	if (!loadFile.is_open()) {
		MessageBox(0, L"Cannot open the shader file!!!", 0, 0);
	}
	size_t fileSize = (size_t)loadFile.tellg();
	std::vector<char> buffer(fileSize);
	loadFile.seekg(0);
	loadFile.read(buffer.data(), fileSize);
	loadFile.close();

	vk::ShaderModuleCreateInfo createInfo = {};
	createInfo.setCodeSize(buffer.size());
	createInfo.setPCode(reinterpret_cast<const uint32_t*>(buffer.data()));

	vk::ShaderModule shaderModule;
	if (device.createShaderModule(&createInfo, 0, &shaderModule) != vk::Result::eSuccess) {
		MessageBox(0, L"Create shader module failed!!!", 0, 0);
	}

	return shaderModule;
}

bool MemoryTypeFromProperties(vk::PhysicalDeviceMemoryProperties memProp, uint32_t typeBits, vk::MemoryPropertyFlags requirementMask, uint32_t& typeIndex) {
	for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
		if ((typeBits & (1 << i)) && (memProp.memoryTypes[i].propertyFlags & requirementMask) == requirementMask) {
			typeIndex = i;
			return true;
		}
	}
	return false;
}

vk::Pipeline CreateGraphicsPipeline(vk::Device device, vk::PipelineDynamicStateCreateInfo dynamic, vk::PipelineVertexInputStateCreateInfo vi, vk::PipelineInputAssemblyStateCreateInfo ia, vk::PipelineRasterizationStateCreateInfo rs, vk::PipelineColorBlendStateCreateInfo cb, vk::PipelineViewportStateCreateInfo vs, vk::PipelineDepthStencilStateCreateInfo ds, vk::PipelineMultisampleStateCreateInfo ms, vk::PipelineLayout layout, std::vector<vk::PipelineShaderStageCreateInfo>& shaders, vk::RenderPass renderPass) {
	auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
		.setLayout(layout)
		.setPColorBlendState(&cb)
		.setPDepthStencilState(&ds)
		.setPDynamicState(&dynamic)
		.setPMultisampleState(&ms)
		.setPRasterizationState(&rs)
		.setStageCount(shaders.size())
		.setPStages(shaders.data())
		.setPViewportState(&vs)
		.setRenderPass(renderPass)
		.setPInputAssemblyState(&ia)
		.setPVertexInputState(&vi);
	vk::Pipeline pipeline;
	device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, 0, &pipeline);
	return pipeline;
}

vk::CommandBuffer BeginSingleTimeCommand(vk::Device* device, const vk::CommandPool& cmdPool) {
	auto cmdAllocInfo = vk::CommandBufferAllocateInfo()
		.setCommandBufferCount(1)
		.setCommandPool(cmdPool)
		.setLevel(vk::CommandBufferLevel::ePrimary);

	vk::CommandBuffer cmd;
	device->allocateCommandBuffers(&cmdAllocInfo, &cmd);

	auto beginInfo = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	cmd.begin(&beginInfo);

	return cmd;
}

void EndSingleTimeCommand(vk::CommandBuffer* cmd, vk::CommandPool& cmdPool, vk::Device* device, vk::Queue* queue) {
	cmd->end();

	auto submitInfo = vk::SubmitInfo()
		.setCommandBufferCount(1)
		.setPCommandBuffers(cmd);

	queue->submit(1, &submitInfo, vk::Fence());
	queue->waitIdle();

	device->freeCommandBuffers(cmdPool, 1, cmd);
}

Attachment CreateAttachment(vk::Device device, vk::PhysicalDeviceMemoryProperties gpuProp, vk::Format format, vk::ImageAspectFlags imageAspect, uint32_t width, uint32_t height, vk::ImageUsageFlags imageUsage) {
	Attachment attachment;

	auto imageInfo = vk::ImageCreateInfo()
		.setArrayLayers(1)
		.setExtent(vk::Extent3D(width, height, 1))
		.setFormat(format)
		.setImageType(vk::ImageType::e2D)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setMipLevels(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(imageUsage);
	device.createImage(&imageInfo, 0, &attachment.image);

	vk::MemoryRequirements sceneImageReqs;
	device.getImageMemoryRequirements(attachment.image, &sceneImageReqs);

	auto memAlloc = vk::MemoryAllocateInfo()
		.setAllocationSize(sceneImageReqs.size);
	MemoryTypeFromProperties(gpuProp, sceneImageReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, memAlloc.memoryTypeIndex);
	device.allocateMemory(&memAlloc, 0, &attachment.memory);

	device.bindImageMemory(attachment.image, attachment.memory, 0);

	auto sceneImageViewInfo = vk::ImageViewCreateInfo()
		.setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA))
		.setFormat(format)
		.setImage(attachment.image)
		.setViewType(vk::ImageViewType::e2D)
		.setSubresourceRange(vk::ImageSubresourceRange(imageAspect, 0, 1, 0, 1));
	device.createImageView(&sceneImageViewInfo, 0, &attachment.imageView);

	return attachment;
}

void DestroyAttachment(vk::Device device, Attachment& attachment) {
	device.destroy(attachment.image);
	device.destroy(attachment.imageView);
	device.free(attachment.memory);
}