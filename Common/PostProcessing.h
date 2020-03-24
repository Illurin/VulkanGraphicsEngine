#pragma once
#include "vkUtil.h"

class PostProcessingProfile {
public:
	struct Bloom {
		float criticalValue = 0.5f;
		float blurOffset = 0.003f;
		int blurRadius = 10;
	};
};

namespace PostProcessing {

	class Bloom {
	public:
		Bloom(PostProcessingProfile::Bloom& profile) {
			bloomProfile = profile;
		}
		void Init(Vulkan* vkInfo, vk::ImageView renderTarget);
		//提取亮度
		void ExtractionBrightness(vk::CommandBuffer* cmd);
		//横向模糊
		void BlurH(vk::CommandBuffer* cmd);
		//纵向模糊
		void BlurV(vk::CommandBuffer* cmd);
		//获取存储Bloom算法结果的图片
		vk::ImageView GetImageView()const {
			return renderView0;
		}
		//提供指向原始颜色贴图的描述符
		vk::DescriptorSet GetSourceDescriptor()const {
			return descSets[0];
		}
		//获取需要的描述符总数（Sampler和SampledImage）
		int GetRequiredDescCount()const {
			return 3;
		}

	private:
		//对外部Vulkan信息的引用
		Vulkan* vkInfo;

		PostProcessingProfile::Bloom bloomProfile;

		//两个渲染目标对应的Image，ImageView以及Framebuffer
		vk::Image renderTarget0;
		vk::Image renderTarget1;
		vk::DeviceMemory renderMemory0;
		vk::DeviceMemory renderMemory1;
		vk::ImageView renderView0;
		vk::ImageView renderView1;
		vk::Framebuffer framebuffer0;
		vk::Framebuffer framebuffer1;

		//存储所有用于计算的图片和采样器的描述符
		std::vector<vk::DescriptorSet> descSets;
		vk::DescriptorSetLayout descSetLayout;
		vk::PipelineLayout pipelineLayout;

		//所有管线
		std::unordered_map<std::string, vk::Pipeline> pipelines;

		//渲染过程
		vk::RenderPass renderPass;

		void PrepareRenderPass();
		void PrepareFramebuffers();
		void PrepareDescriptorSets(vk::ImageView renderTarget);
		void PreparePipelines();
	};

}