#include "ShadowMap.h"

void ShadowMap::Init(vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp, uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;
	
	auto depthImageInfo = vk::ImageCreateInfo()
		.setArrayLayers(1)
		.setExtent(vk::Extent3D(width, height, 1))
		.setFormat(format)
		.setImageType(vk::ImageType::e2D)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setMipLevels(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
	device->createImage(&depthImageInfo, 0, &shadowMap);

	vk::MemoryRequirements depthImageReqs;
	device->getImageMemoryRequirements(shadowMap, &depthImageReqs);

	auto depthMemAlloc = vk::MemoryAllocateInfo()
		.setAllocationSize(depthImageReqs.size);
	MemoryTypeFromProperties(gpuProp, depthImageReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, depthMemAlloc.memoryTypeIndex);
	device->allocateMemory(&depthMemAlloc, 0, &memory);
	device->bindImageMemory(shadowMap, memory, 0);

	auto depthImageViewInfo = vk::ImageViewCreateInfo()
		.setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA))
		.setFormat(format)
		.setImage(shadowMap)
		.setViewType(vk::ImageViewType::e2D)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
	device->createImageView(&depthImageViewInfo, 0, &shadowMapView);
}

void ShadowMap::SetLightTransformMatrix(glm::vec3 lightDirection, float radius) {
	/*View matrix*/
	glm::vec3 lightPos = lightDirection * radius;
	glm::vec3 targetPos = glm::vec3(0.0f);
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	lightView = glm::lookAtRH(lightPos, targetPos, worldUp);

	/*Proj matrix*/
	float width = 50.0f;
	float height = 50.0f;
	float nearZ = -100.0f;
	float farZ = 100.0f;

	lightProj = {
		2.0f / width, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / height, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f / (farZ - nearZ), 0.0f,
		0.0f, 0.0f, nearZ / (nearZ - farZ), 1.0f
	};

	/*Shadow transform matrix*/
	glm::mat4x4 texMatrix = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	};
	shadowTransform = texMatrix * lightProj * lightView;

	lightProj[1][1] *= -1.0f;
}

void ShadowMap::PrepareRenderPass(vk::Device* device) {
	auto description = vk::AttachmentDescription()
		.setFormat(format)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setSamples(vk::SampleCountFlagBits::e1);

	auto reference = vk::AttachmentReference()
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	auto subpass = vk::SubpassDescription()
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(0)
		.setPColorAttachments(0)
		.setPDepthStencilAttachment(&reference);

	auto renderPassInfo = vk::RenderPassCreateInfo()
		.setAttachmentCount(1)
		.setPAttachments(&description)
		.setDependencyCount(0)
		.setPDependencies(0)
		.setSubpassCount(1)
		.setPSubpasses(&subpass);
	device->createRenderPass(&renderPassInfo, 0, &renderPass);
}

void ShadowMap::PrepareFramebuffer(vk::Device* device) {
	auto framebufferInfo = vk::FramebufferCreateInfo()
		.setAttachmentCount(1)
		.setPAttachments(&shadowMapView)
		.setRenderPass(renderPass)
		.setWidth(width)
		.setHeight(height)
		.setLayers(1);
	device->createFramebuffer(&framebufferInfo, 0, &framebuffer);
}

void ShadowMap::BeginRenderPass(vk::CommandBuffer* cmd) {
	vk::ClearValue clearValue = vk::ClearDepthStencilValue(1.0f, 0);
	auto renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setRenderPass(renderPass)
		.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(width, height)))
		.setFramebuffer(framebuffer)
		.setClearValueCount(1)
		.setPClearValues(&clearValue);
	cmd->beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
}