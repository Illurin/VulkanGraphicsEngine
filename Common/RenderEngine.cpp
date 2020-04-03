#include "RenderEngine.h"

void RenderEngine::PrepareResource() {
	/*Create attachments*/
	renderTarget = CreateAttachment(vkInfo->device, vkInfo->gpu.getMemoryProperties(), vk::Format::eR16G16B16A16Sfloat, vk::ImageAspectFlagBits::eColor, vkInfo->width, vkInfo->height, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
	depthTarget = CreateAttachment(vkInfo->device, vkInfo->gpu.getMemoryProperties(), vk::Format::eD16Unorm, vk::ImageAspectFlagBits::eDepth, vkInfo->width, vkInfo->height, vk::ImageUsageFlagBits::eDepthStencilAttachment);

	//第一个管线布局：世界矩阵
	auto objCBBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	vk::DescriptorSetLayoutBinding layoutBindingObject[] = {
		objCBBinding
	};

	//第二个管线布局：纹理，法线贴图，材质常量和采样器
	auto materialCBBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto samplerBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto textureBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(2)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampledImage)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto normalMapBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(3)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampledImage)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	vk::DescriptorSetLayoutBinding layoutBindingMaterial[] = {
		textureBinding, materialCBBinding, samplerBinding, normalMapBinding
	};

	//第三个管线布局：Pass常量和环境立方体图
	auto passCBBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry | vk::ShaderStageFlagBits::eFragment);

	auto ambientCubemap = vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	vk::DescriptorSetLayoutBinding layoutBindingPass[] = {
		passCBBinding, ambientCubemap
	};

	//第四个管线布局：比较采样器和阴影贴图
	auto shadowSamplerBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto shadowMapBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampledImage)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	vk::DescriptorSetLayoutBinding layoutBindingShadow[] = {
		shadowSamplerBinding, shadowMapBinding
	};

	//第五个管线布局：骨骼的变换矩阵
	auto layoutBindingSkinned = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	/*Create descriptor set layout*/
	descSetLayout.resize(5);

	auto descLayoutInfo_obj = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(1)
		.setPBindings(layoutBindingObject);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_obj, 0, &descSetLayout[0]);

	auto descLayoutInfo_material = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(4)
		.setPBindings(layoutBindingMaterial);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_material, 0, &descSetLayout[1]);

	auto descLayoutInfo_pass = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(2)
		.setPBindings(layoutBindingPass);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_pass, 0, &descSetLayout[2]);

	auto descLayoutInfo_shadow = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(2)
		.setPBindings(layoutBindingShadow);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_shadow, 0, &descSetLayout[3]);

	auto descLayoutInfo_skinned = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(1)
		.setPBindings(&layoutBindingSkinned);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_skinned, 0, &descSetLayout[4]);
}

void RenderEngine::PrepareForwardShading() {
	useForwardShading = true;

	/*Create render pass*/
	auto colorAttachment = vk::AttachmentDescription()
		.setFormat(vk::Format::eR16G16B16A16Sfloat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eLoad)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	auto depthAttachment = vk::AttachmentDescription()
		.setFormat(vk::Format::eD16Unorm)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eLoad)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentDescription attachments[] = {
		colorAttachment, depthAttachment
	};

	auto colorReference = vk::AttachmentReference()
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	auto depthReference = vk::AttachmentReference()
		.setAttachment(1)
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	auto subpass = vk::SubpassDescription()
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorReference)
		.setPDepthStencilAttachment(&depthReference);

	auto renderPassInfo = vk::RenderPassCreateInfo()
		.setAttachmentCount(2)
		.setPAttachments(attachments)
		.setSubpassCount(1)
		.setPSubpasses(&subpass);

	vkInfo->device.createRenderPass(&renderPassInfo, 0, &forwardShading.renderPass);

	/*Create framebuffer*/
	vk::ImageView framebufferView[2];
	framebufferView[0] = renderTarget.imageView;
	framebufferView[1] = depthTarget.imageView;
	auto framebufferInfo = vk::FramebufferCreateInfo()
		.setRenderPass(forwardShading.renderPass)
		.setAttachmentCount(2)
		.setPAttachments(framebufferView)
		.setWidth(vkInfo->width)
		.setHeight(vkInfo->height)
		.setLayers(1);
	vkInfo->device.createFramebuffer(&framebufferInfo, 0, &forwardShading.framebuffer);
}

void RenderEngine::PrepareGBuffer() {
	/*Create attachments*/
	vk::PhysicalDeviceMemoryProperties gpuProp = vkInfo->gpu.getMemoryProperties();
	gbuffer.diffuseAttach = CreateAttachment(vkInfo->device, gpuProp, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, vkInfo->width, vkInfo->height, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment);
	gbuffer.normalAttach = CreateAttachment(vkInfo->device, gpuProp, vk::Format::eR32G32B32A32Sfloat, vk::ImageAspectFlagBits::eColor, vkInfo->width, vkInfo->height, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment);
	gbuffer.materialAttach = CreateAttachment(vkInfo->device, gpuProp, vk::Format::eR32G32B32A32Sfloat, vk::ImageAspectFlagBits::eColor, vkInfo->width, vkInfo->height, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment);
	gbuffer.positionAttach = CreateAttachment(vkInfo->device, gpuProp, vk::Format::eR32G32B32A32Sfloat, vk::ImageAspectFlagBits::eColor, vkInfo->width, vkInfo->height, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment);
	gbuffer.shadowPosAttach = CreateAttachment(vkInfo->device, gpuProp, vk::Format::eR32G32B32A32Sfloat, vk::ImageAspectFlagBits::eColor, vkInfo->width, vkInfo->height, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment);
}

void RenderEngine::PrepareDeferredShading() {
	useDeferredShading = true;

	/*Create render pass*/
	std::vector<vk::AttachmentDescription> attachments(7);

	//render target
	attachments[0].setFormat(vk::Format::eR16G16B16A16Sfloat);
	attachments[0].setInitialLayout(vk::ImageLayout::eUndefined);
	attachments[0].setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
	attachments[0].setLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[0].setStoreOp(vk::AttachmentStoreOp::eStore);
	attachments[0].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	attachments[0].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[0].setSamples(vk::SampleCountFlagBits::e1);

	//depth stencil
	attachments[1].setFormat(vk::Format::eD16Unorm);
	attachments[1].setInitialLayout(vk::ImageLayout::eUndefined);
	attachments[1].setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
	attachments[1].setLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[1].setStoreOp(vk::AttachmentStoreOp::eStore);
	attachments[1].setStencilLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[1].setStencilStoreOp(vk::AttachmentStoreOp::eStore);
	attachments[1].setSamples(vk::SampleCountFlagBits::e1);

	//diffuse albedo
	attachments[2].setFormat(vk::Format::eR8G8B8A8Unorm);
	attachments[2].setInitialLayout(vk::ImageLayout::eUndefined);
	attachments[2].setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
	attachments[2].setLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[2].setStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[2].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	attachments[2].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[2].setSamples(vk::SampleCountFlagBits::e1);

	//normal
	attachments[3].setFormat(vk::Format::eR32G32B32A32Sfloat);
	attachments[3].setInitialLayout(vk::ImageLayout::eUndefined);
	attachments[3].setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
	attachments[3].setLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[3].setStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[3].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	attachments[3].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[3].setSamples(vk::SampleCountFlagBits::e1);

	//material properties
	attachments[4].setFormat(vk::Format::eR32G32B32A32Sfloat);
	attachments[4].setInitialLayout(vk::ImageLayout::eUndefined);
	attachments[4].setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
	attachments[4].setLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[4].setStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[4].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	attachments[4].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[4].setSamples(vk::SampleCountFlagBits::e1);

	//position
	attachments[5].setFormat(vk::Format::eR32G32B32A32Sfloat);
	attachments[5].setInitialLayout(vk::ImageLayout::eUndefined);
	attachments[5].setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
	attachments[5].setLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[5].setStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[5].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	attachments[5].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[5].setSamples(vk::SampleCountFlagBits::e1);

	//shadow position
	attachments[6].setFormat(vk::Format::eR32G32B32A32Sfloat);
	attachments[6].setInitialLayout(vk::ImageLayout::eUndefined);
	attachments[6].setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
	attachments[6].setLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[6].setStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[6].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	attachments[6].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[6].setSamples(vk::SampleCountFlagBits::e1);

	vk::AttachmentReference colorReference[5];
	colorReference[0].setAttachment(2);
	colorReference[0].setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	colorReference[1].setAttachment(3);
	colorReference[1].setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	colorReference[2].setAttachment(4);
	colorReference[2].setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	colorReference[3].setAttachment(5);
	colorReference[3].setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	colorReference[4].setAttachment(6);
	colorReference[4].setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference inputReference[5];
	inputReference[0].setAttachment(2);
	inputReference[0].setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	inputReference[1].setAttachment(3);
	inputReference[1].setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	inputReference[2].setAttachment(4);
	inputReference[2].setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	inputReference[3].setAttachment(5);
	inputReference[3].setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	inputReference[4].setAttachment(6);
	inputReference[4].setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	vk::AttachmentReference renderTargetReference;
	renderTargetReference.setAttachment(0);
	renderTargetReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference depthReference;
	depthReference.setAttachment(1);
	depthReference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	std::vector<vk::SubpassDescription> subpassDescriptions(2);
	subpassDescriptions[0].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpassDescriptions[0].setColorAttachmentCount(5);
	subpassDescriptions[0].setPColorAttachments(colorReference);
	subpassDescriptions[0].setPDepthStencilAttachment(&depthReference);

	subpassDescriptions[1].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpassDescriptions[1].setColorAttachmentCount(1);
	subpassDescriptions[1].setPColorAttachments(&renderTargetReference);
	subpassDescriptions[1].setInputAttachmentCount(5);
	subpassDescriptions[1].setPInputAttachments(inputReference);

	std::vector<vk::SubpassDependency> subpassDependencies(3);
	subpassDependencies[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);
	subpassDependencies[0].setDstSubpass(0);
	subpassDependencies[0].setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
	subpassDependencies[0].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
	subpassDependencies[0].setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe);
	subpassDependencies[0].setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependencies[0].setDependencyFlags(vk::DependencyFlagBits::eByRegion);

	subpassDependencies[1].setSrcSubpass(0);
	subpassDependencies[1].setDstSubpass(1);
	subpassDependencies[1].setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
	subpassDependencies[1].setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead);
	subpassDependencies[1].setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependencies[1].setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader);
	subpassDependencies[1].setDependencyFlags(vk::DependencyFlagBits::eByRegion);

	subpassDependencies[2].setSrcSubpass(1);
	subpassDependencies[2].setDstSubpass(VK_SUBPASS_EXTERNAL);
	subpassDependencies[2].setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
	subpassDependencies[2].setDstAccessMask(vk::AccessFlagBits::eMemoryRead);
	subpassDependencies[2].setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependencies[2].setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe);
	subpassDependencies[2].setDependencyFlags(vk::DependencyFlagBits::eByRegion);

	auto renderPassInfo = vk::RenderPassCreateInfo()
		.setAttachmentCount(attachments.size())
		.setPAttachments(attachments.data())
		.setDependencyCount(subpassDependencies.size())
		.setPDependencies(subpassDependencies.data())
		.setSubpassCount(subpassDescriptions.size())
		.setPSubpasses(subpassDescriptions.data());
	vkInfo->device.createRenderPass(&renderPassInfo, 0, &deferredShading.renderPass);

	/*Create framebuffer*/
	vk::ImageView imageViewAttachments[7];
	imageViewAttachments[0] = renderTarget.imageView;
	imageViewAttachments[1] = depthTarget.imageView;
	imageViewAttachments[2] = gbuffer.diffuseAttach.imageView;
	imageViewAttachments[3] = gbuffer.normalAttach.imageView;
	imageViewAttachments[4] = gbuffer.materialAttach.imageView;
	imageViewAttachments[5] = gbuffer.positionAttach.imageView;
	imageViewAttachments[6] = gbuffer.shadowPosAttach.imageView;

	auto framebufferCreateInfo = vk::FramebufferCreateInfo()
		.setAttachmentCount(7)
		.setPAttachments(imageViewAttachments)
		.setLayers(1)
		.setWidth(vkInfo->width)
		.setHeight(vkInfo->height)
		.setRenderPass(deferredShading.renderPass);
	vkInfo->device.createFramebuffer(&framebufferCreateInfo, 0, &deferredShading.framebuffer);
}

void RenderEngine::PrepareDescriptor() {
	if (useDeferredShading) {
		/*Prepare descriptor set layout*/
		std::array<vk::DescriptorSetLayoutBinding, 5> layoutBinding_inputAttach;
		layoutBinding_inputAttach[0].setBinding(0);
		layoutBinding_inputAttach[0].setDescriptorCount(1);
		layoutBinding_inputAttach[0].setDescriptorType(vk::DescriptorType::eInputAttachment);
		layoutBinding_inputAttach[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);
		layoutBinding_inputAttach[1].setBinding(1);
		layoutBinding_inputAttach[1].setDescriptorCount(1);
		layoutBinding_inputAttach[1].setDescriptorType(vk::DescriptorType::eInputAttachment);
		layoutBinding_inputAttach[1].setStageFlags(vk::ShaderStageFlagBits::eFragment);
		layoutBinding_inputAttach[2].setBinding(2);
		layoutBinding_inputAttach[2].setDescriptorCount(1);
		layoutBinding_inputAttach[2].setDescriptorType(vk::DescriptorType::eInputAttachment);
		layoutBinding_inputAttach[2].setStageFlags(vk::ShaderStageFlagBits::eFragment);
		layoutBinding_inputAttach[3].setBinding(3);
		layoutBinding_inputAttach[3].setDescriptorCount(1);
		layoutBinding_inputAttach[3].setDescriptorType(vk::DescriptorType::eInputAttachment);
		layoutBinding_inputAttach[3].setStageFlags(vk::ShaderStageFlagBits::eFragment);
		layoutBinding_inputAttach[4].setBinding(4);
		layoutBinding_inputAttach[4].setDescriptorCount(1);
		layoutBinding_inputAttach[4].setDescriptorType(vk::DescriptorType::eInputAttachment);
		layoutBinding_inputAttach[4].setStageFlags(vk::ShaderStageFlagBits::eFragment);
		auto descSetLayoutInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(layoutBinding_inputAttach.size())
			.setPBindings(layoutBinding_inputAttach.data());
		vkInfo->device.createDescriptorSetLayout(&descSetLayoutInfo, 0, &gbuffer.descSetLayout);

		/*Allocate descriptor set*/
		auto descSetAlloc = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(vkInfo->descPool)
			.setDescriptorSetCount(1)
			.setPSetLayouts(&gbuffer.descSetLayout);
		vkInfo->device.allocateDescriptorSets(&descSetAlloc, &gbuffer.descSet);

		/*Update descriptor set*/
		std::array<vk::WriteDescriptorSet, 5> updateInfo;

		auto diffuseAttachInfo = vk::DescriptorImageInfo()
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(gbuffer.diffuseAttach.imageView);
		updateInfo[0] = vk::WriteDescriptorSet()
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eInputAttachment)
			.setDstArrayElement(0)
			.setDstBinding(0)
			.setDstSet(gbuffer.descSet)
			.setPImageInfo(&diffuseAttachInfo);

		auto normalAttachInfo = vk::DescriptorImageInfo()
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(gbuffer.normalAttach.imageView);
		updateInfo[1] = vk::WriteDescriptorSet()
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eInputAttachment)
			.setDstArrayElement(0)
			.setDstBinding(1)
			.setDstSet(gbuffer.descSet)
			.setPImageInfo(&normalAttachInfo);

		auto materialAttachInfo = vk::DescriptorImageInfo()
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(gbuffer.materialAttach.imageView);
		updateInfo[2] = vk::WriteDescriptorSet()
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eInputAttachment)
			.setDstArrayElement(0)
			.setDstBinding(2)
			.setDstSet(gbuffer.descSet)
			.setPImageInfo(&materialAttachInfo);

		auto positionAttachInfo = vk::DescriptorImageInfo()
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(gbuffer.positionAttach.imageView);
		updateInfo[3] = vk::WriteDescriptorSet()
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eInputAttachment)
			.setDstArrayElement(0)
			.setDstBinding(3)
			.setDstSet(gbuffer.descSet)
			.setPImageInfo(&positionAttachInfo);

		auto shadowPosAttachInfo = vk::DescriptorImageInfo()
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(gbuffer.shadowPosAttach.imageView);
		updateInfo[4] = vk::WriteDescriptorSet()
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eInputAttachment)
			.setDstArrayElement(0)
			.setDstBinding(4)
			.setDstSet(gbuffer.descSet)
			.setPImageInfo(&shadowPosAttachInfo);

		vkInfo->device.updateDescriptorSets(updateInfo.size(), updateInfo.data(), 0, 0);

		/*Create pipeline layout*/
		std::array<vk::DescriptorSetLayout, 4> pipelineDescSet = {
			descSetLayout[0], gbuffer.descSetLayout, descSetLayout[2], descSetLayout[3]
		};
		auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(pipelineDescSet.size())
			.setPSetLayouts(pipelineDescSet.data());
		vkInfo->device.createPipelineLayout(&pipelineLayoutInfo, 0, &deferredShading.pipelineLayout);
	}
}

void RenderEngine::PreparePipeline() {
	if (useDeferredShading) {
		auto vertexShader = CreateShaderModule("Shaders\\vertex.spv", vkInfo->device);
		auto outputShader = CreateShaderModule("Shaders\\deferredShadingOutput.spv", vkInfo->device);
		auto quadShader = CreateShaderModule("Shaders\\bloomVS.spv", vkInfo->device);
		auto processingShader = CreateShaderModule("Shaders\\deferredShadingProcessing.spv", vkInfo->device);

		std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderInfo(2);

		pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
			.setPName("main")
			.setModule(vertexShader)
			.setStage(vk::ShaderStageFlagBits::eVertex);

		pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
			.setPName("main")
			.setModule(outputShader)
			.setStage(vk::ShaderStageFlagBits::eFragment);

		//Dynamic state
		auto dynamicInfo = vk::PipelineDynamicStateCreateInfo();
		std::vector<vk::DynamicState> dynamicStates;

		//Vertex input state
		auto viInfo = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptionCount(1)
			.setPVertexBindingDescriptions(&vkInfo->vertex.binding)
			.setVertexAttributeDescriptionCount(vkInfo->vertex.attrib.size())
			.setPVertexAttributeDescriptions(vkInfo->vertex.attrib.data());

		//Input assembly state
		auto iaInfo = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(VK_FALSE);

		//Rasterization state
		auto rsInfo = vk::PipelineRasterizationStateCreateInfo()
			.setCullMode(vk::CullModeFlagBits::eNone)
			.setDepthBiasEnable(VK_FALSE)
			.setDepthClampEnable(VK_FALSE)
			.setFrontFace(vk::FrontFace::eClockwise)
			.setLineWidth(1.0f)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setRasterizerDiscardEnable(VK_FALSE);

		//Color blend state
		std::vector<vk::PipelineColorBlendAttachmentState> attState(5);
		for (auto& att : attState) {
			att = vk::PipelineColorBlendAttachmentState()
				.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
		}
		
		auto cbInfo = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(VK_FALSE)
			.setAttachmentCount(attState.size())
			.setPAttachments(attState.data())
			.setLogicOp(vk::LogicOp::eNoOp);

		//Viewport state
		vk::Viewport viewport;
		viewport.setMaxDepth(1.0f);
		viewport.setMinDepth(0.0f);
		viewport.setX(0.0f);
		viewport.setY(0.0f);
		viewport.setWidth(vkInfo->width);
		viewport.setHeight(vkInfo->height);

		auto scissor = vk::Rect2D()
			.setOffset(vk::Offset2D(0.0f, 0.0f))
			.setExtent(vk::Extent2D(vkInfo->width, vkInfo->height));

		auto vpInfo = vk::PipelineViewportStateCreateInfo()
			.setScissorCount(1)
			.setPScissors(&scissor)
			.setViewportCount(1)
			.setPViewports(&viewport);

		//Depth stencil state
		auto dsInfo = vk::PipelineDepthStencilStateCreateInfo()
			.setDepthTestEnable(VK_TRUE)
			.setDepthWriteEnable(VK_TRUE)
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthBoundsTestEnable(VK_FALSE)
			.setStencilTestEnable(VK_FALSE);

		//Multisample state
		auto msInfo = vk::PipelineMultisampleStateCreateInfo()
			.setAlphaToCoverageEnable(VK_FALSE)
			.setAlphaToOneEnable(VK_FALSE)
			.setMinSampleShading(0.0f)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setSampleShadingEnable(VK_FALSE);

		auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
			.setLayout(vkInfo->pipelineLayout["scene"])
			.setPColorBlendState(&cbInfo)
			.setPDepthStencilState(&dsInfo)
			.setPDynamicState(&dynamicInfo)
			.setPMultisampleState(&msInfo)
			.setPRasterizationState(&rsInfo)
			.setStageCount(pipelineShaderInfo.size())
			.setPStages(pipelineShaderInfo.data())
			.setPViewportState(&vpInfo)
			.setRenderPass(deferredShading.renderPass)
			.setPInputAssemblyState(&iaInfo)
			.setPVertexInputState(&viInfo);

		auto shaderModelSME = vk::SpecializationMapEntry()
			.setConstantID(0)
			.setOffset(0)
			.setSize(sizeof(int));

		deferredShading.outputPipeline.resize((int)ShaderModel::shaderModelCount);
		for (int i = 0; i < (int)ShaderModel::shaderModelCount; i++) {
			auto shaderModelSI = vk::SpecializationInfo()
				.setDataSize(sizeof(int))
				.setMapEntryCount(1)
				.setPMapEntries(&shaderModelSME)
				.setPData(&i);

			pipelineShaderInfo[1].setPSpecializationInfo(&shaderModelSI);

			vkInfo->device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, 0, &deferredShading.outputPipeline[i]);
		}

		pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
			.setPName("main")
			.setModule(quadShader)
			.setStage(vk::ShaderStageFlagBits::eVertex);

		pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
			.setPName("main")
			.setModule(processingShader)
			.setStage(vk::ShaderStageFlagBits::eFragment);

		iaInfo = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleStrip)
			.setPrimitiveRestartEnable(VK_FALSE);

		viInfo = vk::PipelineVertexInputStateCreateInfo();

		cbInfo = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(VK_FALSE)
			.setAttachmentCount(1)
			.setPAttachments(attState.data())
			.setLogicOp(vk::LogicOp::eNoOp);
		
		pipelineInfo.setSubpass(1);
		pipelineInfo.setLayout(deferredShading.pipelineLayout);
		
		vkInfo->device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, 0, &deferredShading.processingPipeline);

		vkInfo->device.destroy(vertexShader);
		vkInfo->device.destroy(outputShader);
		vkInfo->device.destroy(quadShader);
		vkInfo->device.destroy(processingShader);
	}
}

void RenderEngine::BeginForwardShading(vk::CommandBuffer cmd) {
	if (!useForwardShading) {
		MessageBox(0, L"You dont use forward shading!", 0, 0);
		return;
	}

	vk::ClearValue clearValue[2] = {
		vk::ClearColorValue(std::array<float, 4>({0.0f, 0.0f, 0.0f, 0.0f})),
		vk::ClearDepthStencilValue(1.0f, 0)
	};
	vk::RenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.setClearValueCount(2);
	renderPassBeginInfo.setPClearValues(clearValue);
	renderPassBeginInfo.setFramebuffer(forwardShading.framebuffer);
	renderPassBeginInfo.setRenderPass(forwardShading.renderPass);
	renderPassBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(0.0f, 0.0f), vk::Extent2D(vkInfo->width, vkInfo->height)));
	cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
}

void RenderEngine::BeginDeferredShading(vk::CommandBuffer cmd) {
	if (!useDeferredShading) {
		MessageBox(0, L"You dont use deferred shading!", 0, 0);
		return;
	}

	vk::ClearValue clearValue[7];
	clearValue[0].setColor(vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f })));
	clearValue[1].setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0.0f));
	clearValue[2].setColor(vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f })));
	clearValue[3].setColor(vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f })));
	clearValue[4].setColor(vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f })));
	clearValue[5].setColor(vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f })));
	clearValue[6].setColor(vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f })));
	auto renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setClearValueCount(7)
		.setPClearValues(clearValue)
		.setFramebuffer(deferredShading.framebuffer)
		.setRenderPass(deferredShading.renderPass)
		.setRenderArea(vk::Rect2D(vk::Offset2D(0.0f, 0.0f), vk::Extent2D(vkInfo->width, vkInfo->height)));
	cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
}