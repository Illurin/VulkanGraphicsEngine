#pragma once
#include "vkUtil.h"

class PostProcessingProfile {
public:
	struct Bloom {
		float criticalValue = 0.5f;
		float blurOffset = 0.003f;
		int blurRadius = 10;
	};

	struct HDR {
		float exposure = 1.0f;
	};
};

namespace PostProcessing {

	class Bloom {
	public:
		Bloom(PostProcessingProfile::Bloom& profile) {
			bloomProfile = profile;
		}
		~Bloom() {
			DestroyAttachment(vkInfo->device, renderTarget0);
			DestroyAttachment(vkInfo->device, renderTarget1);
			vkInfo->device.destroy(sampler);
			vkInfo->device.destroy(bloomRenderPass);
			vkInfo->device.destroy(combineRenderPass);
			for (auto& framebuffer : bloomFramebuffers) {
				vkInfo->device.destroy(framebuffer);
			}
			for (auto& framebuffer : combineFramebuffers) {
				vkInfo->device.destroy(framebuffer);
			}
			for (auto& layout : descSetLayout) {
				vkInfo->device.destroy(layout);
			}
			for (auto& layout : pipelineLayout) {
				vkInfo->device.destroy(layout);
			}
			for (auto& pipeline : pipelines) {
				vkInfo->device.destroy(pipeline.second);
			}
		}

		void SetHDRProperties(float exposure);

		vk::RenderPass GetRenderPass()const { return combineRenderPass; }

		void Begin(vk::CommandBuffer cmd, uint32_t currentImage);

		//对外部Vulkan信息的引用
		Vulkan* vkInfo;

		void PrepareRenderPass();
		void PrepareFramebuffers();
		void PrepareDescriptorSets(vk::ImageView sourceImage);
		void PreparePipelines();

	private:
		PostProcessingProfile::Bloom bloomProfile;
		std::unique_ptr<Buffer<PostProcessingProfile::HDR>> hdrProperties;

		//两个渲染目标对应的Image，ImageView以及Framebuffer
		Attachment renderTarget0;
		Attachment renderTarget1;

		std::vector<vk::Framebuffer> bloomFramebuffers;
		std::vector<vk::Framebuffer> combineFramebuffers;

		vk::Sampler sampler;

		//存储所有用于计算的图片和采样器的描述符
		std::vector<vk::DescriptorSet> descSets;
		std::vector<vk::DescriptorSetLayout> descSetLayout;
		std::vector<vk::PipelineLayout> pipelineLayout;

		//所有管线
		std::unordered_map<std::string, vk::Pipeline> pipelines;

		//渲染过程
		vk::RenderPass bloomRenderPass;
		vk::RenderPass combineRenderPass;
	};

}