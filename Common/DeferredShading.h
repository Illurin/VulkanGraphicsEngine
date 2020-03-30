#pragma once
#include "vkUtil.h"

class DeferredShading {
public:
	DeferredShading(Vulkan* vkInfo) {
		this->vkInfo = vkInfo;
		PrepareAttachments();
		CreateRenderPass();
		CreateFramebuffer();
		PreparePipeline();
	}
	~DeferredShading() {
		vkInfo->device.destroy(renderPass);
		vkInfo->device.destroy(framebuffer);
		vkInfo->device.destroy(outputPipeline);
		vkInfo->device.destroy(processingPipeline);
		
		DestroyAttachment(vkInfo->device, depthAttach);
		DestroyAttachment(vkInfo->device, normalAttach);
		DestroyAttachment(vkInfo->device, positionAttach);
		DestroyAttachment(vkInfo->device, materialAttach);
	}

	void PrepareAttachments();
	void CreateRenderPass();
	void CreateFramebuffer();
	void PreparePipeline();

	void Begin(vk::CommandBuffer cmd);
	void NextSubpass(vk::CommandBuffer cmd);

private:
	Vulkan* vkInfo;

	vk::RenderPass renderPass;
	vk::Framebuffer framebuffer;
	vk::Pipeline outputPipeline;
	vk::Pipeline processingPipeline;

	Attachment depthAttach;
	Attachment normalAttach;
	Attachment materialAttach;
	Attachment positionAttach;
	Attachment shadowPosAttach;
};