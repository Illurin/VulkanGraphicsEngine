#include "PipelineManagement.h"

void PipelineManagement::SetDefaultPipeline(vk::PipelineDynamicStateCreateInfo dynamic, vk::PipelineVertexInputStateCreateInfo vi, vk::PipelineInputAssemblyStateCreateInfo ia, vk::PipelineRasterizationStateCreateInfo rs, vk::PipelineColorBlendStateCreateInfo cb, vk::PipelineViewportStateCreateInfo vs, vk::PipelineDepthStencilStateCreateInfo ds, vk::PipelineMultisampleStateCreateInfo ms, vk::PipelineLayout layout, vk::RenderPass renderPass) {
	defaultPipeline.dynamicState = dynamic;
	defaultPipeline.colorBlend = cb;
	defaultPipeline.depthStencil = ds;
	defaultPipeline.inputAssembly = ia;
	defaultPipeline.multisample = ms;
	defaultPipeline.pipelineLayout = layout;
	defaultPipeline.rasterization = rs;
	defaultPipeline.renderPass = renderPass;
	defaultPipeline.vertexInput = vi;
	defaultPipeline.viewport = vs;
}

void PipelineManagement::ClearToDefault() {
	currentPipeline.dynamicState = defaultPipeline.dynamicState;
	currentPipeline.colorBlend = defaultPipeline.colorBlend;
	currentPipeline.depthStencil = defaultPipeline.depthStencil;
	currentPipeline.inputAssembly = defaultPipeline.inputAssembly;
	currentPipeline.multisample = defaultPipeline.multisample;
	currentPipeline.pipelineLayout = defaultPipeline.pipelineLayout;
	currentPipeline.rasterization = defaultPipeline.rasterization;
	currentPipeline.renderPass = defaultPipeline.renderPass;
	currentPipeline.vertexInput = defaultPipeline.vertexInput;
	currentPipeline.viewport = defaultPipeline.viewport;
}

void PipelineManagement::SetVertexInput(vk::VertexInputBindingDescription binding, std::vector<vk::VertexInputAttributeDescription>& attrib) {
	currentPipeline.vertexInput.setVertexAttributeDescriptionCount(attrib.size());
	currentPipeline.vertexInput.setPVertexAttributeDescriptions(attrib.data());
	currentPipeline.vertexInput.setVertexBindingDescriptionCount(1);
	currentPipeline.vertexInput.setPVertexBindingDescriptions(&binding);
}

void PipelineManagement::SetColorBlend(std::vector<vk::PipelineColorBlendAttachmentState>& attach, std::array<float, 4Ui64>& blendConstants) {
	currentPipeline.colorBlend.setAttachmentCount(attach.size());
	currentPipeline.colorBlend.setPAttachments(attach.data());
	currentPipeline.colorBlend.setBlendConstants(blendConstants);
}

void PipelineManagement::SetInputAssembly(vk::PrimitiveTopology topology) {
	currentPipeline.inputAssembly.setTopology(topology);
}

void PipelineManagement::SetDepthBias(float clamp, float constantFactor, float slopeFactor) {
	currentPipeline.rasterization.setDepthBiasEnable(VK_TRUE);
	currentPipeline.rasterization.setDepthBiasClamp(clamp);
	currentPipeline.rasterization.setDepthBiasConstantFactor(constantFactor);
	currentPipeline.rasterization.setDepthBiasSlopeFactor(slopeFactor);
}

vk::Pipeline PipelineManagement::CreatePipeline(vk::Device device, std::vector<vk::PipelineShaderStageCreateInfo>& shaders) {
	auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
		.setLayout(currentPipeline.pipelineLayout)
		.setPColorBlendState(&currentPipeline.colorBlend)
		.setPDepthStencilState(&currentPipeline.depthStencil)
		.setPDynamicState(&currentPipeline.dynamicState)
		.setPMultisampleState(&currentPipeline.multisample)
		.setPRasterizationState(&currentPipeline.rasterization)
		.setStageCount(shaders.size())
		.setPStages(shaders.data())
		.setPViewportState(&currentPipeline.viewport)
		.setRenderPass(currentPipeline.renderPass)
		.setPInputAssemblyState(&currentPipeline.inputAssembly)
		.setPVertexInputState(&currentPipeline.vertexInput);
	vk::Pipeline pipeline;
	device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, 0, &pipeline);
	return pipeline;
}