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