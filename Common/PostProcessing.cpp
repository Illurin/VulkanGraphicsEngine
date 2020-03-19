#include "PostProcessing.h"

void PostProcessing::Bloom::PrepareRenderPass() {
	auto colorAttachment = vk::AttachmentDescription()
		.setFormat(vkInfo->format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	auto colorReference = vk::AttachmentReference()
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	auto subpass = vk::SubpassDescription()
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorReference)
		.setPDepthStencilAttachment(0);

	auto renderPassInfo = vk::RenderPassCreateInfo()
		.setAttachmentCount(1)
		.setPAttachments(&colorAttachment)
		.setSubpassCount(1)
		.setPSubpasses(&subpass);

	vkInfo->device.createRenderPass(&renderPassInfo, 0, &renderPass);
}

void PostProcessing::Bloom::PrepareFramebuffers() {
	auto imageInfo = vk::ImageCreateInfo()
		.setArrayLayers(1)
		.setExtent(vk::Extent3D(vkInfo->width, vkInfo->height, 1))
		.setFormat(vkInfo->format)
		.setImageType(vk::ImageType::e2D)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setMipLevels(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
	vkInfo->device.createImage(&imageInfo, 0, &renderTarget0);
	vkInfo->device.createImage(&imageInfo, 0, &renderTarget1);

	{
		vk::MemoryRequirements renderTargetReqs;
		vkInfo->device.getImageMemoryRequirements(renderTarget0, &renderTargetReqs);

		auto renderTargetMemAlloc = vk::MemoryAllocateInfo()
			.setAllocationSize(renderTargetReqs.size);
		MemoryTypeFromProperties(vkInfo->gpu.getMemoryProperties(), renderTargetReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, renderTargetMemAlloc.memoryTypeIndex);
		vkInfo->device.allocateMemory(&renderTargetMemAlloc, 0, &renderMemory0);

		vkInfo->device.bindImageMemory(renderTarget0, renderMemory0, 0);
	}
	{
		vk::MemoryRequirements renderTargetReqs;
		vkInfo->device.getImageMemoryRequirements(renderTarget1, &renderTargetReqs);

		auto renderTargetMemAlloc = vk::MemoryAllocateInfo()
			.setAllocationSize(renderTargetReqs.size);
		MemoryTypeFromProperties(vkInfo->gpu.getMemoryProperties(), renderTargetReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, renderTargetMemAlloc.memoryTypeIndex);
		vkInfo->device.allocateMemory(&renderTargetMemAlloc, 0, &renderMemory1);

		vkInfo->device.bindImageMemory(renderTarget1, renderMemory1, 0);
	}
	auto renderTargetViewInfo = vk::ImageViewCreateInfo()
		.setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA))
		.setFormat(vkInfo->format)
		.setImage(renderTarget0)
		.setViewType(vk::ImageViewType::e2D)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
	vkInfo->device.createImageView(&renderTargetViewInfo, 0, &renderView0);
	renderTargetViewInfo.setImage(renderTarget1);
	vkInfo->device.createImageView(&renderTargetViewInfo, 0, &renderView1);

	auto framebufferInfo = vk::FramebufferCreateInfo()
		.setRenderPass(renderPass)
		.setAttachmentCount(1)
		.setPAttachments(&renderView0)
		.setWidth(vkInfo->width)
		.setHeight(vkInfo->height)
		.setLayers(1);
	vkInfo->device.createFramebuffer(&framebufferInfo, 0, &framebuffer0);
	framebufferInfo.setPAttachments(&renderView1);
	vkInfo->device.createFramebuffer(&framebufferInfo, 0, &framebuffer1);
}

void PostProcessing::Bloom::PrepareDescriptorSets(vk::ImageView renderTarget) {
	//创建描述符布局
	vk::DescriptorSetLayoutBinding layoutbinding[2];
	layoutbinding[0] = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	layoutbinding[1] = vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampledImage)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto descLayoutInfo = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(2)
		.setPBindings(layoutbinding);

	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo, 0, &descSetLayout);

	//分配描述符
	vk::DescriptorSetLayout layouts[] = {
		descSetLayout, descSetLayout, descSetLayout
	};

	auto descSetAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(vkInfo->descPool)
		.setDescriptorSetCount(3)
		.setPSetLayouts(layouts);

	descSets.resize(3);
	vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, descSets.data());

	//更新描述符
	vk::Sampler sampler;
	{
		auto samplerInfo = vk::SamplerCreateInfo()
			.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
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
			.setUnnormalizedCoordinates(VK_FALSE);
		vkInfo->device.createSampler(&samplerInfo, 0, &sampler);
	}

	auto descriptorSamplerInfo = vk::DescriptorImageInfo()
		.setSampler(sampler);

	vk::ImageView descriptorImageView[] = {
		renderTarget, renderView0, renderView1
	};

	for (uint32_t i = 0; i < descSets.size(); i++) {
		auto descriptorImageInfo = vk::DescriptorImageInfo()
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(descriptorImageView[i]);

		vk::WriteDescriptorSet descSetWrites[2];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eSampler);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(0);
		descSetWrites[0].setDstSet(descSets[i]);
		descSetWrites[0].setPImageInfo(&descriptorSamplerInfo);
		descSetWrites[1].setDescriptorCount(1);
		descSetWrites[1].setDescriptorType(vk::DescriptorType::eSampledImage);
		descSetWrites[1].setDstArrayElement(0);
		descSetWrites[1].setDstBinding(1);
		descSetWrites[1].setDstSet(descSets[i]);
		descSetWrites[1].setPImageInfo(&descriptorImageInfo);
		vkInfo->device.updateDescriptorSets(2, descSetWrites, 0, 0);
	}
}

void PostProcessing::Bloom::PreparePipelines() {
	auto vsModule = CreateShaderModule("Shaders\\bloomVS.spv", vkInfo->device);
	auto psModule = CreateShaderModule("Shaders\\bloomPS.spv", vkInfo->device);
	auto blurH = CreateShaderModule("Shaders\\blurH.spv", vkInfo->device);
	auto blurV = CreateShaderModule("Shaders\\blurV.spv", vkInfo->device);

	//编译着色器
	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderInfo(2);

	pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(vsModule)
		.setStage(vk::ShaderStageFlagBits::eVertex);

	pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(psModule)
		.setStage(vk::ShaderStageFlagBits::eFragment);

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
		.setPSetLayouts(&descSetLayout);

	vkInfo->device.createPipelineLayout(&plInfo, 0, &pipelineLayout);

	//16.11 Create pipeline state
	auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
		.setLayout(pipelineLayout)
		.setPColorBlendState(&cbInfo)
		.setPDepthStencilState(&dsInfo)
		.setPDynamicState(&dynamicInfo)
		.setPMultisampleState(&msInfo)
		.setPRasterizationState(&rsInfo)
		.setStageCount(2)
		.setPStages(pipelineShaderInfo.data())
		.setPViewportState(&vpInfo)
		.setRenderPass(renderPass)
		.setPInputAssemblyState(&iaInfo)
		.setPVertexInputState(&viInfo);

	vkInfo->device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, nullptr, &pipelines["brightness"]);

	pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(blurH)
		.setStage(vk::ShaderStageFlagBits::eFragment);

	vkInfo->device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, nullptr, &pipelines["blurH"]);

	pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(blurV)
		.setStage(vk::ShaderStageFlagBits::eFragment);

	vkInfo->device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, nullptr, &pipelines["blurV"]);

	vkInfo->device.destroyShaderModule(vsModule);
	vkInfo->device.destroyShaderModule(psModule);
	vkInfo->device.destroyShaderModule(blurH);
	vkInfo->device.destroyShaderModule(blurV);
}

void PostProcessing::Bloom::Init(Vulkan& vkInfo, vk::ImageView renderTarget) {
	this->vkInfo = &vkInfo;
	PrepareRenderPass();
	PrepareFramebuffers();
	PrepareDescriptorSets(renderTarget);
	PreparePipelines();
}

void PostProcessing::Bloom::ExtractionBrightness(vk::CommandBuffer* cmd) {
	vk::ClearValue clearValue = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f }));

	auto renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setFramebuffer(framebuffer0)
		.setRenderArea(vk::Rect2D(vk::Offset2D(0.0f, 0.0f), vk::Extent2D(vkInfo->width, vkInfo->height)))
		.setRenderPass(renderPass)
		.setClearValueCount(1)
		.setPClearValues(&clearValue);

	cmd->beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

	cmd->bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines["brightness"]);
	cmd->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descSets[0], 0, 0);

	cmd->draw(4, 1, 0, 0);

	cmd->endRenderPass();
}

void PostProcessing::Bloom::BlurH(vk::CommandBuffer* cmd) {
	vk::ClearValue clearValue = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f }));

	auto renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setFramebuffer(framebuffer1)
		.setRenderArea(vk::Rect2D(vk::Offset2D(0.0f, 0.0f), vk::Extent2D(vkInfo->width, vkInfo->height)))
		.setRenderPass(renderPass)
		.setClearValueCount(1)
		.setPClearValues(&clearValue);

	cmd->beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

	cmd->bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines["blurH"]);
	cmd->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descSets[1], 0, 0);

	cmd->draw(4, 1, 0, 0);

	cmd->endRenderPass();
}

void PostProcessing::Bloom::BlurV(vk::CommandBuffer* cmd) {
	vk::ClearValue clearValue = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f }));

	auto renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setFramebuffer(framebuffer0)
		.setRenderArea(vk::Rect2D(vk::Offset2D(0.0f, 0.0f), vk::Extent2D(vkInfo->width, vkInfo->height)))
		.setRenderPass(renderPass)
		.setClearValueCount(1)
		.setPClearValues(&clearValue);

	cmd->beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

	cmd->bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines["blurV"]);
	cmd->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descSets[2], 0, 0);

	cmd->draw(4, 1, 0, 0);

	cmd->endRenderPass();
}