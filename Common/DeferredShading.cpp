#include "DeferredShading.h"

void DeferredShading::PrepareAttachments() {
	vk::PhysicalDeviceMemoryProperties gpuProp = vkInfo->gpu.getMemoryProperties();
	depthAttach = CreateAttachment(vkInfo->device, gpuProp, vkInfo->depth.format, vk::ImageAspectFlagBits::eDepth, vkInfo->width, vkInfo->height, vk::ImageUsageFlagBits::eDepthStencilAttachment);
	normalAttach = CreateAttachment(vkInfo->device, gpuProp, vk::Format::eR32G32B32A32Sfloat, vk::ImageAspectFlagBits::eColor, vkInfo->width, vkInfo->height, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment);
	materialAttach = CreateAttachment(vkInfo->device, gpuProp, vk::Format::eR32G32B32A32Sfloat, vk::ImageAspectFlagBits::eColor, vkInfo->width, vkInfo->height, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment);
	positionAttach = CreateAttachment(vkInfo->device, gpuProp, vk::Format::eR32G32B32A32Sfloat, vk::ImageAspectFlagBits::eColor, vkInfo->width, vkInfo->height, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment);
	shadowPosAttach = CreateAttachment(vkInfo->device, gpuProp, vk::Format::eR32G32B32A32Sfloat, vk::ImageAspectFlagBits::eColor, vkInfo->width, vkInfo->height, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment);
}

void DeferredShading::CreateRenderPass() {
	std::vector<vk::AttachmentDescription> attachments(6);

	//render target
	attachments[0].setFormat(vkInfo->format);
	attachments[0].setInitialLayout(vk::ImageLayout::eUndefined);
	attachments[0].setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	attachments[0].setLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[0].setStoreOp(vk::AttachmentStoreOp::eStore);
	attachments[0].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	attachments[0].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[0].setSamples(vk::SampleCountFlagBits::e1);

	//depth stencil
	attachments[1].setFormat(vkInfo->depth.format);
	attachments[1].setInitialLayout(vk::ImageLayout::eUndefined);
	attachments[1].setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
	attachments[1].setLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[1].setStoreOp(vk::AttachmentStoreOp::eStore);
	attachments[1].setStencilLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[1].setStencilStoreOp(vk::AttachmentStoreOp::eStore);
	attachments[1].setSamples(vk::SampleCountFlagBits::e1);

	//normal
	attachments[2].setFormat(vk::Format::eR32G32B32A32Sfloat);
	attachments[2].setInitialLayout(vk::ImageLayout::eUndefined);
	attachments[2].setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
	attachments[2].setLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[2].setStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[2].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	attachments[2].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[2].setSamples(vk::SampleCountFlagBits::e1);

	//material properties
	attachments[3].setFormat(vk::Format::eR32G32B32A32Sfloat);
	attachments[3].setInitialLayout(vk::ImageLayout::eUndefined);
	attachments[3].setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
	attachments[3].setLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[3].setStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[3].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	attachments[3].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[3].setSamples(vk::SampleCountFlagBits::e1);

	//position
	attachments[4].setFormat(vk::Format::eR32G32B32A32Sfloat);
	attachments[4].setInitialLayout(vk::ImageLayout::eUndefined);
	attachments[4].setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
	attachments[4].setLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[4].setStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[4].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	attachments[4].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[4].setSamples(vk::SampleCountFlagBits::e1);

	//shadow position
	attachments[5].setFormat(vk::Format::eR32G32B32A32Sfloat);
	attachments[5].setInitialLayout(vk::ImageLayout::eUndefined);
	attachments[5].setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
	attachments[5].setLoadOp(vk::AttachmentLoadOp::eClear);
	attachments[5].setStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[5].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	attachments[5].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachments[5].setSamples(vk::SampleCountFlagBits::e1);

	vk::AttachmentReference colorReference[4];
	colorReference[0].setAttachment(2);
	colorReference[0].setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	colorReference[1].setAttachment(3);
	colorReference[1].setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	colorReference[2].setAttachment(4);
	colorReference[2].setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	colorReference[3].setAttachment(5);
	colorReference[3].setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference renderTargetReference;
	renderTargetReference.setAttachment(0);
	renderTargetReference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference depthReference;
	depthReference.setAttachment(1);
	depthReference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	std::vector<vk::SubpassDescription> subpassDescriptions(2);
	subpassDescriptions[0].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpassDescriptions[0].setColorAttachmentCount(1);
	subpassDescriptions[0].setPColorAttachments(&renderTargetReference);
	subpassDescriptions[0].setPDepthStencilAttachment(&depthReference);

	subpassDescriptions[1].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpassDescriptions[1].setColorAttachmentCount(1);
	subpassDescriptions[1].setPColorAttachments(&renderTargetReference);
	subpassDescriptions[1].setInputAttachmentCount(4);
	subpassDescriptions[1].setPInputAttachments(colorReference);

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
		//.setDependencyCount(subpassDependencies.size())
		//.setPDependencies(subpassDependencies.data())
		.setSubpassCount(1)
		.setPSubpasses(subpassDescriptions.data());
	vkInfo->device.createRenderPass(&renderPassInfo, 0, &renderPass);
}

void DeferredShading::CreateFramebuffer() {
	vk::ImageView attachments[6];
	attachments[0] = vkInfo->scene.imageView;
	attachments[1] = depthAttach.imageView;
	attachments[2] = normalAttach.imageView;
	attachments[3] = materialAttach.imageView;
	attachments[4] = positionAttach.imageView;
	attachments[5] = shadowPosAttach.imageView;

	auto framebufferCreateInfo = vk::FramebufferCreateInfo()
		.setAttachmentCount(6)
		.setPAttachments(attachments)
		.setLayers(1)
		.setWidth(vkInfo->width)
		.setHeight(vkInfo->height)
		.setRenderPass(renderPass);
	vkInfo->device.createFramebuffer(&framebufferCreateInfo, 0, &framebuffer);
}

void DeferredShading::PreparePipeline() {
	/*Create pipelines*/
	auto vertexShader = CreateShaderModule("Shaders\\vertex.spv", vkInfo->device);
	auto outputShader = CreateShaderModule("Shaders\\deferredShadingOutput.spv", vkInfo->device);
	auto quadShader = CreateShaderModule("Shaders\\bloomVS.spv", vkInfo->device);
	auto processingShader = CreateShaderModule("Shaders\\fragment.spv", vkInfo->device);

	//Create pipeline shader module
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
	std::vector<vk::PipelineColorBlendAttachmentState> attState(4);

	auto cbInfo = vk::PipelineColorBlendStateCreateInfo()
		.setLogicOpEnable(VK_FALSE)
		.setAttachmentCount(1)
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
		.setDepthTestEnable(VK_FALSE)
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
		.setRenderPass(renderPass)
		.setPInputAssemblyState(&iaInfo)
		.setPVertexInputState(&viInfo);
	vkInfo->device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, 0, &outputPipeline);

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

	cbInfo = vk::PipelineColorBlendStateCreateInfo()
		.setLogicOpEnable(VK_FALSE)
		.setAttachmentCount(1)
		.setPAttachments(attState.data())
		.setLogicOp(vk::LogicOp::eNoOp);

	pipelineInfo.setSubpass(1);

	//vkInfo->device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, 0, &processingPipeline);

	vkInfo->device.destroy(vertexShader);
	vkInfo->device.destroy(outputShader);
	vkInfo->device.destroy(quadShader);
	vkInfo->device.destroy(processingShader);
}

void DeferredShading::Begin(vk::CommandBuffer cmd) {
	vk::ClearValue clearValue[6];
	clearValue[0].setColor(vk::ClearColorValue(std::array<float, 4>({ 1.0f, 0.0f, 0.0f, 0.0f })));
	clearValue[1].setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0.0f));
	clearValue[2].setColor(vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f })));
	clearValue[3].setColor(vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f })));
	clearValue[4].setColor(vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f })));
	clearValue[5].setColor(vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f })));
	auto renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setClearValueCount(6)
		.setPClearValues(clearValue)
		.setFramebuffer(framebuffer)
		.setRenderPass(renderPass)
		.setRenderArea(vk::Rect2D(vk::Offset2D(0.0f, 0.0f), vk::Extent2D(vkInfo->width, vkInfo->height)));
	cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, outputPipeline);
}

void DeferredShading::NextSubpass(vk::CommandBuffer cmd) {
	cmd.nextSubpass(vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, processingPipeline);
}