#pragma once
#include "vkUtil.h"

class PipelineManagement {
public:
	void SetDefaultPipeline(vk::PipelineDynamicStateCreateInfo, vk::PipelineVertexInputStateCreateInfo, vk::PipelineInputAssemblyStateCreateInfo, vk::PipelineRasterizationStateCreateInfo, vk::PipelineColorBlendStateCreateInfo, vk::PipelineViewportStateCreateInfo, vk::PipelineDepthStencilStateCreateInfo, vk::PipelineMultisampleStateCreateInfo, vk::PipelineLayout, vk::RenderPass);
	void SetVertexInput(vk::VertexInputBindingDescription binding, std::vector<vk::VertexInputAttributeDescription>& attrib);
	void SetColorBlend(std::vector<vk::PipelineColorBlendAttachmentState>& attach, std::array<float, 4Ui64>& blendConstants);
	void SetInputAssembly(vk::PrimitiveTopology topology);
	void SetDepthBias(float clamp, float constantFactor, float slopeFactor);
	void ClearToDefault();
	vk::Pipeline CreatePipeline(vk::Device device, std::vector<vk::PipelineShaderStageCreateInfo>& shaders);

private:
	struct {
		vk::PipelineDynamicStateCreateInfo dynamicState;
		vk::PipelineVertexInputStateCreateInfo vertexInput;
		vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
		vk::PipelineRasterizationStateCreateInfo rasterization;
		vk::PipelineColorBlendStateCreateInfo colorBlend;
		vk::PipelineViewportStateCreateInfo viewport;
		vk::PipelineDepthStencilStateCreateInfo depthStencil;
		vk::PipelineMultisampleStateCreateInfo multisample;
		vk::PipelineLayout pipelineLayout;
		vk::RenderPass renderPass;
	}defaultPipeline;

	struct {
		vk::PipelineDynamicStateCreateInfo dynamicState;
		vk::PipelineVertexInputStateCreateInfo vertexInput;
		vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
		vk::PipelineRasterizationStateCreateInfo rasterization;
		vk::PipelineColorBlendStateCreateInfo colorBlend;
		vk::PipelineViewportStateCreateInfo viewport;
		vk::PipelineDepthStencilStateCreateInfo depthStencil;
		vk::PipelineMultisampleStateCreateInfo multisample;
		vk::PipelineLayout pipelineLayout;
		vk::RenderPass renderPass;
	}currentPipeline;
};