#include "PostProcessing.h"

void PostProcessing::Bloom::SetHDRProperties(float exposure, float gamma) {
	PostProcessingProfile::HDR hdrProfile;
	hdrProfile.exposure = exposure;
	hdrProfile.gamma = gamma;

	hdrProperties->CopyData(&vkInfo->device, 0, 1, &hdrProfile);
}

void PostProcessing::Bloom::PrepareRenderPass() {
	{
		auto attachment = vk::AttachmentDescription()
			.setFormat(vk::Format::eR16G16B16A16Sfloat)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::AttachmentReference colorReference;
		colorReference.setAttachment(0);
		colorReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		auto subpassDescription = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&colorReference);

		auto renderPassInfo = vk::RenderPassCreateInfo()
			.setAttachmentCount(1)
			.setPAttachments(&attachment)
			.setSubpassCount(1)
			.setPSubpasses(&subpassDescription);

		vkInfo->device.createRenderPass(&renderPassInfo, 0, &bloomRenderPass);
	}
	{
		auto attachment = vk::AttachmentDescription()
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		vk::AttachmentReference colorReference;
		colorReference.setAttachment(0);
		colorReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		auto subpassDescription = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&colorReference);

		auto renderPassInfo = vk::RenderPassCreateInfo()
			.setAttachmentCount(1)
			.setPAttachments(&attachment)
			.setSubpassCount(1)
			.setPSubpasses(&subpassDescription);

		vkInfo->device.createRenderPass(&renderPassInfo, 0, &combineRenderPass);
	}
}

void PostProcessing::Bloom::PrepareFramebuffers() {
	renderTarget0 = CreateAttachment(vkInfo->device, vkInfo->gpu.getMemoryProperties(), vk::Format::eR16G16B16A16Sfloat, vk::ImageAspectFlagBits::eColor, vkInfo->width, vkInfo->height, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled);
	renderTarget1 = CreateAttachment(vkInfo->device, vkInfo->gpu.getMemoryProperties(), vk::Format::eR16G16B16A16Sfloat, vk::ImageAspectFlagBits::eColor, vkInfo->width, vkInfo->height, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);

	vk::ImageView attachment;

	auto framebufferInfo = vk::FramebufferCreateInfo()
		.setAttachmentCount(1)
		.setPAttachments(&attachment)
		.setWidth(vkInfo->width)
		.setHeight(vkInfo->height)
		.setLayers(1);

	framebufferInfo.setRenderPass(bloomRenderPass);
	bloomFramebuffers.resize(2);

	attachment = renderTarget0.imageView;
	vkInfo->device.createFramebuffer(&framebufferInfo, 0, &bloomFramebuffers[0]);

	attachment = renderTarget1.imageView;
	vkInfo->device.createFramebuffer(&framebufferInfo, 0, &bloomFramebuffers[1]);

	combineFramebuffers.resize(vkInfo->frameCount);

	framebufferInfo.setRenderPass(combineRenderPass);
	for (uint32_t i = 0; i < vkInfo->frameCount; i++) {
		attachment = vkInfo->swapchainImageViews[i];
		vkInfo->device.createFramebuffer(&framebufferInfo, 0, &combineFramebuffers[i]);
	}
}

void PostProcessing::Bloom::PrepareDescriptorSets(vk::ImageView sourceImage) {
	//创建描述符布局
	descSetLayout.resize(2);

	vk::DescriptorSetLayoutBinding layoutbinding_combine[3];
	layoutbinding_combine[0] = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	layoutbinding_combine[1] = vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	layoutbinding_combine[2] = vk::DescriptorSetLayoutBinding()
		.setBinding(2)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto descLayoutInfo = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(3)
		.setPBindings(layoutbinding_combine);

	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo, 0, &descSetLayout[1]);

	vk::DescriptorSetLayoutBinding layoutbinding_sampleImage;
	layoutbinding_sampleImage = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	descLayoutInfo = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(1)
		.setPBindings(&layoutbinding_sampleImage);

	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo, 0, &descSetLayout[0]);

	//分配描述符
	descSets.resize(4);

	auto descSetAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(vkInfo->descPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&descSetLayout[0]);
	vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, &descSets[0]);

	descSetAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(vkInfo->descPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&descSetLayout[0]);
	vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, &descSets[1]);

	descSetAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(vkInfo->descPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&descSetLayout[0]);
	vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, &descSets[2]);

	descSetAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(vkInfo->descPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&descSetLayout[1]);
	vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, &descSets[3]);

	//创建采样器
	auto samplerInfo = vk::SamplerCreateInfo()
		.setAnisotropyEnable(VK_FALSE)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMagFilter(vk::Filter::eLinear)
		.setMaxLod(1.0f)
		.setMinLod(0.0f)
		.setMipLodBias(0.0f)
		.setMinFilter(vk::Filter::eLinear)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeW(vk::SamplerAddressMode::eClampToEdge);
	vkInfo->device.createSampler(&samplerInfo, 0, &sampler);

	hdrProperties = std::make_unique<Buffer<PostProcessingProfile::HDR>>(&vkInfo->device, 1, vk::BufferUsageFlagBits::eUniformBuffer, vkInfo->gpu.getMemoryProperties(), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, true);

	//更新描述符
	std::array<vk::WriteDescriptorSet, 6> updateInfo;

	updateInfo[0].setDescriptorCount(1);
	updateInfo[0].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	updateInfo[0].setDstArrayElement(0);
	updateInfo[0].setDstBinding(0);
	updateInfo[0].setDstSet(descSets[0]);
	updateInfo[0].setPImageInfo(&vk::DescriptorImageInfo(sampler, sourceImage, vk::ImageLayout::eShaderReadOnlyOptimal));

	updateInfo[1].setDescriptorCount(1);
	updateInfo[1].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	updateInfo[1].setDstArrayElement(0);
	updateInfo[1].setDstBinding(0);
	updateInfo[1].setDstSet(descSets[1]);
	updateInfo[1].setPImageInfo(&vk::DescriptorImageInfo(sampler, renderTarget0.imageView, vk::ImageLayout::eShaderReadOnlyOptimal));

	updateInfo[2].setDescriptorCount(1);
	updateInfo[2].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	updateInfo[2].setDstArrayElement(0);
	updateInfo[2].setDstBinding(0);
	updateInfo[2].setDstSet(descSets[2]);
	updateInfo[2].setPImageInfo(&vk::DescriptorImageInfo(sampler, renderTarget1.imageView, vk::ImageLayout::eShaderReadOnlyOptimal));

	updateInfo[3].setDescriptorCount(1);
	updateInfo[3].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	updateInfo[3].setDstArrayElement(0);
	updateInfo[3].setDstBinding(1);
	updateInfo[3].setDstSet(descSets[3]);
	updateInfo[3].setPImageInfo(&vk::DescriptorImageInfo(sampler, sourceImage, vk::ImageLayout::eShaderReadOnlyOptimal));

	updateInfo[4].setDescriptorCount(1);
	updateInfo[4].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	updateInfo[4].setDstArrayElement(0);
	updateInfo[4].setDstBinding(2);
	updateInfo[4].setDstSet(descSets[3]);
	updateInfo[4].setPImageInfo(&vk::DescriptorImageInfo(sampler, renderTarget0.imageView, vk::ImageLayout::eShaderReadOnlyOptimal));

	updateInfo[5].setDescriptorCount(1);
	updateInfo[5].setDescriptorType(vk::DescriptorType::eUniformBuffer);
	updateInfo[5].setDstArrayElement(0);
	updateInfo[5].setDstBinding(0);
	updateInfo[5].setDstSet(descSets[3]);
	updateInfo[5].setPBufferInfo(&vk::DescriptorBufferInfo(hdrProperties->GetBuffer(), 0, sizeof(PostProcessingProfile::HDR)));

	vkInfo->device.updateDescriptorSets(updateInfo.size(), updateInfo.data(), 0, 0);
}

void PostProcessing::Bloom::PreparePipelines() {
	auto vsModule = CreateShaderModule("Shaders\\bloomVS.spv", vkInfo->device);
	auto psModule = CreateShaderModule("Shaders\\bloomPS.spv", vkInfo->device);
	auto blurH = CreateShaderModule("Shaders\\blurH.spv", vkInfo->device);
	auto blurV = CreateShaderModule("Shaders\\blurV.spv", vkInfo->device);
	auto combineShader = CreateShaderModule("Shaders\\combine.spv", vkInfo->device);

	pipelineLayout.resize(2);

	//编译着色器
	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderInfo(2);

	vk::SpecializationMapEntry bloomPSSME;
	bloomPSSME.setConstantID(0);
	bloomPSSME.setOffset(0);
	bloomPSSME.setSize(sizeof(float));

	vk::SpecializationInfo bloomPSSI;
	bloomPSSI.setDataSize(sizeof(PostProcessingProfile::Bloom));
	bloomPSSI.setPData(&bloomProfile);
	bloomPSSI.setMapEntryCount(1);
	bloomPSSI.setPMapEntries(&bloomPSSME);
	
	pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(vsModule)
		.setStage(vk::ShaderStageFlagBits::eVertex);

	pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(psModule)
		.setStage(vk::ShaderStageFlagBits::eFragment)
		.setPSpecializationInfo(&bloomPSSI);

	//16.1 Dynamic state
	auto dynamicInfo = vk::PipelineDynamicStateCreateInfo();
	std::vector<vk::DynamicState> dynamicStates;

	//16.2 Vertex input state
	auto viInfo = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptionCount(0)
		.setPVertexBindingDescriptions(0)
		.setVertexAttributeDescriptionCount(0)
		.setPVertexAttributeDescriptions(0);

	//16.3 Input assembly state
	auto iaInfo = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(vk::PrimitiveTopology::eTriangleStrip)
		.setPrimitiveRestartEnable(VK_FALSE);

	//16.4 Rasterization state
	auto rsInfo = vk::PipelineRasterizationStateCreateInfo()
		.setCullMode(vk::CullModeFlagBits::eNone)
		.setDepthBiasEnable(VK_FALSE)
		.setDepthClampEnable(VK_FALSE)
		.setFrontFace(vk::FrontFace::eClockwise)
		.setLineWidth(1.0f)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setRasterizerDiscardEnable(VK_FALSE);

	//16.5 Color blend state
	auto attState = vk::PipelineColorBlendAttachmentState()
		.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
		.setBlendEnable(VK_FALSE)
		.setAlphaBlendOp(vk::BlendOp::eAdd)
		.setColorBlendOp(vk::BlendOp::eAdd);

	auto cbInfo = vk::PipelineColorBlendStateCreateInfo()
		.setLogicOpEnable(VK_FALSE)
		.setAttachmentCount(1)
		.setPAttachments(&attState)
		.setLogicOp(vk::LogicOp::eNoOp);

	//16.6 Viewport state
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

	//16.7 Depth stencil state
	auto dsInfo = vk::PipelineDepthStencilStateCreateInfo()
		.setDepthTestEnable(VK_FALSE)
		.setDepthWriteEnable(VK_FALSE)
		.setDepthCompareOp(vk::CompareOp::eAlways)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setStencilTestEnable(VK_FALSE);

	//16.8 Multisample state
	auto msInfo = vk::PipelineMultisampleStateCreateInfo()
		.setAlphaToCoverageEnable(VK_FALSE)
		.setAlphaToOneEnable(VK_FALSE)
		.setMinSampleShading(0.0f)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		.setSampleShadingEnable(VK_FALSE);

	//16.9 Pipeline layout
	auto plInfo = vk::PipelineLayoutCreateInfo()
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(0)
		.setSetLayoutCount(1)
		.setPSetLayouts(&descSetLayout[0]);
	vkInfo->device.createPipelineLayout(&plInfo, 0, &pipelineLayout[0]);

	//16.11 Create pipeline state
	auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
		.setLayout(pipelineLayout[0])
		.setPColorBlendState(&cbInfo)
		.setPDepthStencilState(&dsInfo)
		.setPDynamicState(&dynamicInfo)
		.setPMultisampleState(&msInfo)
		.setPRasterizationState(&rsInfo)
		.setStageCount(2)
		.setPStages(pipelineShaderInfo.data())
		.setPViewportState(&vpInfo)
		.setRenderPass(bloomRenderPass)
		.setPInputAssemblyState(&iaInfo)
		.setPVertexInputState(&viInfo);

	vkInfo->device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, nullptr, &pipelines["brightness"]);

	std::array<vk::SpecializationMapEntry, 2> blurSME;
	blurSME[0].setConstantID(0);
	blurSME[0].setOffset(sizeof(float));
	blurSME[0].setSize(sizeof(float));
	blurSME[1].setConstantID(1);
	blurSME[1].setOffset(2 * sizeof(float));
	blurSME[1].setSize(sizeof(int));

	vk::SpecializationInfo blurSI;
	blurSI.setDataSize(sizeof(PostProcessingProfile::Bloom));
	blurSI.setPData(&bloomProfile);
	blurSI.setMapEntryCount(blurSME.size());
	blurSI.setPMapEntries(blurSME.data());

	pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(blurH)
		.setStage(vk::ShaderStageFlagBits::eFragment)
		.setPSpecializationInfo(&blurSI);

	vkInfo->device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, nullptr, &pipelines["blurH"]);

	pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(blurV)
		.setStage(vk::ShaderStageFlagBits::eFragment)
		.setPSpecializationInfo(&blurSI);

	vkInfo->device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, nullptr, &pipelines["blurV"]);

	//编译用于图像混合的着色器
	pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(combineShader)
		.setStage(vk::ShaderStageFlagBits::eFragment);

	//创建管线布局
	auto combinePipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(0)
		.setSetLayoutCount(1)
		.setPSetLayouts(&descSetLayout[1]);
	vkInfo->device.createPipelineLayout(&combinePipelineLayoutInfo, 0, &pipelineLayout[1]);

	//创建用于图像混合的管线
	pipelineInfo.setLayout(pipelineLayout[1]);
	pipelineInfo.setRenderPass(combineRenderPass);

	vkInfo->device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, nullptr, &pipelines["combine"]);

	vkInfo->device.destroyShaderModule(vsModule);
	vkInfo->device.destroyShaderModule(psModule);
	vkInfo->device.destroyShaderModule(blurH);
	vkInfo->device.destroyShaderModule(blurV);
	vkInfo->device.destroyShaderModule(combineShader);
}

void PostProcessing::Bloom::Begin(vk::CommandBuffer cmd, uint32_t currentImage) {
	vk::ClearValue clearValue[] = {
		vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f }))
	};

	auto renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setFramebuffer(bloomFramebuffers[0])
		.setRenderArea(vk::Rect2D(vk::Offset2D(0.0f, 0.0f), vk::Extent2D(vkInfo->width, vkInfo->height)))
		.setRenderPass(bloomRenderPass)
		.setClearValueCount(1)
		.setPClearValues(clearValue);
	cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines["brightness"]);
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout[0], 0, 1, &descSets[0], 0, 0);
	cmd.draw(4, 1, 0, 0);
	cmd.endRenderPass();

	renderPassBeginInfo.setFramebuffer(bloomFramebuffers[1]);
	cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines["blurH"]);
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout[0], 0, 1, &descSets[1], 0, 0);
	cmd.draw(4, 1, 0, 0);
	cmd.endRenderPass();

	renderPassBeginInfo.setFramebuffer(bloomFramebuffers[0]);
	cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines["blurV"]);
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout[0], 0, 1, &descSets[2], 0, 0);
	cmd.draw(4, 1, 0, 0);
	cmd.endRenderPass();

	renderPassBeginInfo.setFramebuffer(combineFramebuffers[currentImage]);
	renderPassBeginInfo.setRenderPass(combineRenderPass);
	cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines["combine"]);
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout[1], 0, 1, &descSets[3], 0, 0);
	cmd.draw(4, 1, 0, 0);
}