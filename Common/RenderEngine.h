#pragma once
#include "vkUtil.h"
#include "Component.h"

class RenderEngine {
public:
	RenderEngine() {}
	~RenderEngine() {
		DestroyAttachment(vkInfo->device, renderTarget);
		DestroyAttachment(vkInfo->device, depthTarget);

		for (auto& layout : descSetLayout) {
			vkInfo->device.destroy(layout);
		}

		if (useForwardShading) {
			vkInfo->device.destroy(forwardShading.framebuffer);
			vkInfo->device.destroy(forwardShading.renderPass);
		}
		
		if (useDeferredShading) {
			vkInfo->device.destroy(deferredShading.framebuffer);
			vkInfo->device.destroy(deferredShading.renderPass);
			vkInfo->device.destroy(deferredShading.processingPipeline);
			vkInfo->device.destroy(deferredShading.pipelineLayout);
			vkInfo->device.destroy(gbuffer.descSetLayout);
			for (auto pipeline : deferredShading.outputPipeline) {
				vkInfo->device.destroy(pipeline);
			}
		}
	}

	void PrepareResource();
	void PrepareForwardShading();
	void PrepareGBuffer();
	void PrepareDeferredShading();

	void PrepareDescriptor();
	void PreparePipeline();

	void BeginForwardShading(vk::CommandBuffer cmd);
	void BeginDeferredShading(vk::CommandBuffer cmd);

	Vulkan* vkInfo;

	Attachment renderTarget;
	Attachment depthTarget;

	std::vector<vk::DescriptorSetLayout> descSetLayout;

	struct {
		vk::Framebuffer framebuffer;
		vk::RenderPass renderPass;
	}forwardShading;

	struct {
		Attachment diffuseAttach;
		Attachment normalAttach;
		Attachment materialAttach;
		Attachment positionAttach;
		Attachment shadowPosAttach;
		vk::DescriptorSetLayout descSetLayout;
		vk::DescriptorSet descSet;
	}gbuffer;

	struct {
		vk::Framebuffer framebuffer;
		vk::RenderPass renderPass;
		vk::PipelineLayout pipelineLayout;
		std::vector<vk::Pipeline> outputPipeline;
		vk::Pipeline processingPipeline;
	}deferredShading;

	bool useForwardShading = false;
	bool useDeferredShading = false;
};