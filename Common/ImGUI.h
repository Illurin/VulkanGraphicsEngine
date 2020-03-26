#pragma once
#include "vkUtil.h"
#include "imgui/imgui.h"

/*
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;

void main()
{
	Out.Color = aColor;
	Out.UV = aUV;
	gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
*/
static uint32_t imguiVertexShaderSpv[] =
{
	0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
	0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
	0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
	0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
	0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
	0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
	0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
	0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
	0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
	0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
	0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
	0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
	0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
	0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
	0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
	0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
	0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
	0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
	0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
	0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
	0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
	0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
	0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
	0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
	0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
	0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
	0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
	0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
	0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
	0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
	0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
	0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
	0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
	0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
	0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
	0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
	0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
	0x0000002d,0x0000002c,0x000100fd,0x00010038
};

/*
#version 450 core
layout(location = 0) out vec4 fColor;
layout(set=0, binding=0) uniform sampler2D sTexture;
layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
void main()
{
	fColor = In.Color * texture(sTexture, In.UV.st);
}
*/
static uint32_t imguiFragmentShaderSpv[] =
{
	0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
	0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
	0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
	0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000016,0x78655473,0x65727574,
	0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
	0x00000000,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
	0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
	0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
	0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
	0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
	0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
	0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
	0x00000007,0x00090019,0x00000013,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
	0x00000001,0x00000000,0x0003001b,0x00000014,0x00000013,0x00040020,0x00000015,0x00000000,
	0x00000014,0x0004003b,0x00000015,0x00000016,0x00000000,0x0004002b,0x0000000e,0x00000018,
	0x00000001,0x00040020,0x00000019,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,
	0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,
	0x0000000f,0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000014,0x00000017,
	0x00000016,0x00050041,0x00000019,0x0000001a,0x0000000d,0x00000018,0x0004003d,0x0000000a,
	0x0000001b,0x0000001a,0x00050057,0x00000007,0x0000001c,0x00000017,0x0000001b,0x00050085,
	0x00000007,0x0000001d,0x00000012,0x0000001c,0x0003003e,0x00000009,0x0000001d,0x000100fd,
	0x00010038
};

class ImGUI {
private:
	Vulkan* vkInfo;

	vk::Sampler sampler;
	std::unique_ptr<Buffer<ImDrawVert>> vertexBuffer = nullptr;
	uint32_t vertexCount = 0;
	std::unique_ptr<Buffer<ImDrawIdx>> indexBuffer = nullptr;
	uint32_t indexCount = 0;

	vk::Image fontImage;
	vk::DeviceMemory fontMemory;
	vk::ImageView fontView;

	vk::Pipeline pipeline;
	vk::PipelineLayout pipelineLayout;

	vk::DescriptorPool descPool;
	vk::DescriptorSetLayout descSetLayout;
	vk::DescriptorSet descSet;

public:
	struct PushConstant {
		glm::vec2 scale;
		glm::vec2 translate;
	}pushConstantBlock;

	ImGUI(Vulkan* vkInfo) {
		this->vkInfo = vkInfo;
		ImGui::CreateContext();
	}

	~ImGUI() {
		ImGui::DestroyContext();
		vkInfo->device.destroyImage(fontImage, 0);
		vkInfo->device.destroyImageView(fontView, 0);
		vkInfo->device.freeMemory(fontMemory, 0);
		vkInfo->device.destroySampler(sampler, 0);
		vkInfo->device.destroyPipeline(pipeline, 0);
		vkInfo->device.destroyPipelineLayout(pipelineLayout, 0);
		vkInfo->device.destroyDescriptorPool(descPool, 0);
		vkInfo->device.destroyDescriptorSetLayout(descSetLayout, 0);
	}

	void Init(float width, float height) {
		// Color scheme
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
		// Dimensions
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(width, height);
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	}

	void InitResource() {
		ImGuiIO& io = ImGui::GetIO();

		// Create font texture
		unsigned char* fontData;
		int texWidth, texHeight;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
		VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

		// Create target image for copy
		auto imageInfo = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setExtent(vk::Extent3D(texWidth, texHeight, 1))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
			.setInitialLayout(vk::ImageLayout::eUndefined);
		vkInfo->device.createImage(&imageInfo, nullptr, &fontImage);

		vk::MemoryRequirements memReqs;
		vkInfo->device.getImageMemoryRequirements(fontImage, &memReqs);
		auto memAllocInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(memReqs.size);
		MemoryTypeFromProperties(vkInfo->gpu.getMemoryProperties(), memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, memAllocInfo.memoryTypeIndex);
		vkInfo->device.allocateMemory(&memAllocInfo, 0, &fontMemory);
		vkInfo->device.bindImageMemory(fontImage, fontMemory, 0);

		// Image view
		auto viewInfo = vk::ImageViewCreateInfo()
			.setImage(fontImage)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(vk::Format::eR8G8B8A8Unorm)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
		vkInfo->device.createImageView(&viewInfo, nullptr, &fontView);

		// Staging buffers for font data upload
		vk::Buffer stagingBuffer;
		vk::DeviceMemory bufferMemory;

		auto bufferInfo = vk::BufferCreateInfo()
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
			.setSize(uploadSize);
		vkInfo->device.createBuffer(&bufferInfo, 0, &stagingBuffer);

		//Allocate the memory of buffer
		vk::MemoryRequirements uploaderMemReqs;
		vkInfo->device.getBufferMemoryRequirements(stagingBuffer, &uploaderMemReqs);

		auto bufferMemoryInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(uploaderMemReqs.size);
		MemoryTypeFromProperties(vkInfo->gpu.getMemoryProperties(), uploaderMemReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, bufferMemoryInfo.memoryTypeIndex);
		vkInfo->device.allocateMemory(&bufferMemoryInfo, 0, &bufferMemory);

		vkInfo->device.bindBufferMemory(stagingBuffer, bufferMemory, 0);

		BYTE* mapPtr;
		vkInfo->device.mapMemory(bufferMemory, 0, uploadSize, vk::MemoryMapFlags(), reinterpret_cast<void**>(&mapPtr));
		memcpy(mapPtr, fontData, uploadSize);
		vkInfo->device.unmapMemory(bufferMemory);

		// Copy buffer data to font image
		vk::CommandBuffer copyCmd = BeginSingleTimeCommand(&vkInfo->device, vkInfo->cmdPool);

		auto subresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);

		// Prepare for transfer
		auto barrier = vk::ImageMemoryBarrier()
			.setImage(fontImage)
			.setOldLayout(vk::ImageLayout::eUndefined)
			.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setSubresourceRange(subresourceRange)
			.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
		copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, 0, 0, 0, 1, &barrier);

		// Copy
		auto copyRegion = vk::BufferImageCopy()
			.setBufferOffset(0)
			.setBufferImageHeight(0)
			.setBufferRowLength(0)
			.setImageOffset(vk::Offset3D(0, 0, 0))
			.setImageExtent(vk::Extent3D(texWidth, texHeight, 1))
			.setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1));

		copyCmd.copyBufferToImage(stagingBuffer, fontImage, vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

		// Prepare for shader read
		barrier = vk::ImageMemoryBarrier()
			.setImage(fontImage)
			.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
			.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setSubresourceRange(subresourceRange)
			.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
			.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		copyCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(), 0, 0, 0, 0, 1, &barrier);

		EndSingleTimeCommand(&copyCmd, vkInfo->cmdPool, &vkInfo->device, &vkInfo->queue);

		vkInfo->device.destroyBuffer(stagingBuffer);
		vkInfo->device.freeMemory(bufferMemory);

		// Font texture Sampler
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

		// Descriptor pool
		vk::DescriptorPoolSize typeCount[1];
		typeCount[0].setType(vk::DescriptorType::eCombinedImageSampler);
		typeCount[0].setDescriptorCount(1);

		auto descriptorPoolInfo = vk::DescriptorPoolCreateInfo()
			.setMaxSets(1)
			.setPoolSizeCount(1)
			.setPPoolSizes(typeCount);
		vkInfo->device.createDescriptorPool(&descriptorPoolInfo, 0, &vkInfo->descPool);

		// Descriptor set layout
		auto textureBinding = vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment);

		auto descLayoutInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(1)
			.setPBindings(&textureBinding);
		vkInfo->device.createDescriptorSetLayout(&descLayoutInfo, 0, &descSetLayout);

		// Descriptor set
		auto descSetAllocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(vkInfo->descPool)
			.setDescriptorSetCount(1)
			.setPSetLayouts(&descSetLayout);
		vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, &descSet);

		auto descriptorUpdateInfo = vk::DescriptorImageInfo()
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(fontView)
			.setSampler(sampler);

		vk::WriteDescriptorSet descSetWrites[1];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(0);
		descSetWrites[0].setDstSet(descSet);
		descSetWrites[0].setPImageInfo(&descriptorUpdateInfo);
		vkInfo->device.updateDescriptorSets(1, descSetWrites, 0, 0);

		// Pipeline layout
		// Push constants for UI rendering parameters
		auto pushConstantRange = vk::PushConstantRange()
			.setOffset(0)
			.setSize(sizeof(PushConstant))
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);

		auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(1)
			.setPSetLayouts(&descSetLayout)
			.setPushConstantRangeCount(1)
			.setPPushConstantRanges(&pushConstantRange);

		vkInfo->device.createPipelineLayout(&pipelineLayoutInfo, 0, &pipelineLayout);

		// Setup graphics pipeline for UI rendering

		//Create pipeline shader module
		vk::ShaderModule vsModule;
		vk::ShaderModule psModule;

		auto shaderModuleInfo = vk::ShaderModuleCreateInfo()
			.setCodeSize(sizeof(imguiVertexShaderSpv))
			.setPCode(imguiVertexShaderSpv);
		vkInfo->device.createShaderModule(&shaderModuleInfo, 0, &vsModule);

		shaderModuleInfo = vk::ShaderModuleCreateInfo()
			.setCodeSize(sizeof(imguiFragmentShaderSpv))
			.setPCode(imguiFragmentShaderSpv);
		vkInfo->device.createShaderModule(&shaderModuleInfo, 0, &psModule);

		std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderInfo(2);

		pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
			.setPName("main")
			.setModule(vsModule)
			.setStage(vk::ShaderStageFlagBits::eVertex);

		pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
			.setPName("main")
			.setModule(psModule)
			.setStage(vk::ShaderStageFlagBits::eFragment);

		//Dynamic state
		std::vector<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};
		auto dynamicInfo = vk::PipelineDynamicStateCreateInfo()
			.setDynamicStateCount(dynamicStates.size())
			.setPDynamicStates(dynamicStates.data());

		//Input assembly state
		auto iaInfo = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(VK_FALSE);

		//Rasterization state
		auto rsInfo = vk::PipelineRasterizationStateCreateInfo()
			.setCullMode(vk::CullModeFlagBits::eNone)
			.setDepthBiasEnable(VK_FALSE)
			.setDepthClampEnable(VK_FALSE)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setLineWidth(1.0f)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setRasterizerDiscardEnable(VK_FALSE);

		//Color blend state
		auto attState = vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
			.setBlendEnable(VK_TRUE)
			.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
			.setAlphaBlendOp(vk::BlendOp::eAdd);

		auto cbInfo = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(VK_FALSE)
			.setAttachmentCount(1)
			.setPAttachments(&attState)
			.setLogicOp(vk::LogicOp::eNoOp);

		//Viewport state
		auto vpInfo = vk::PipelineViewportStateCreateInfo()
			.setScissorCount(1)
			.setViewportCount(1);

		//Depth stencil state
		auto dsInfo = vk::PipelineDepthStencilStateCreateInfo()
			.setDepthTestEnable(VK_FALSE)
			.setDepthWriteEnable(VK_FALSE)
			.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
			.setDepthBoundsTestEnable(VK_FALSE)
			.setStencilTestEnable(VK_FALSE);

		//Multisample state
		auto msInfo = vk::PipelineMultisampleStateCreateInfo()
			.setAlphaToCoverageEnable(VK_FALSE)
			.setAlphaToOneEnable(VK_FALSE)
			.setMinSampleShading(0.0f)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setSampleShadingEnable(VK_FALSE);

		// Vertex bindings an attributes based on ImGui vertex definition
		auto vertexInputBinding = vk::VertexInputBindingDescription()
			.setBinding(0)
			.setInputRate(vk::VertexInputRate::eVertex)
			.setStride(sizeof(ImDrawVert));

		std::array<vk::VertexInputAttributeDescription, 3> vertexInputAttributes;
		vertexInputAttributes[0].setBinding(0);
		vertexInputAttributes[0].setLocation(0);
		vertexInputAttributes[0].setFormat(vk::Format::eR32G32Sfloat);
		vertexInputAttributes[0].setOffset(offsetof(ImDrawVert, pos));
		vertexInputAttributes[1].setBinding(0);
		vertexInputAttributes[1].setLocation(1);
		vertexInputAttributes[1].setFormat(vk::Format::eR32G32Sfloat);
		vertexInputAttributes[1].setOffset(offsetof(ImDrawVert, uv));
		vertexInputAttributes[2].setBinding(0);
		vertexInputAttributes[2].setLocation(2);
		vertexInputAttributes[2].setFormat(vk::Format::eR8G8B8A8Unorm);
		vertexInputAttributes[2].setOffset(offsetof(ImDrawVert, col));

		//Vertex input state
		auto viInfo = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptionCount(1)
			.setPVertexBindingDescriptions(&vertexInputBinding)
			.setVertexAttributeDescriptionCount(vertexInputAttributes.size())
			.setPVertexAttributeDescriptions(vertexInputAttributes.data());

		//Create pipeline state
		pipeline = CreateGraphicsPipeline(vkInfo->device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, pipelineLayout, pipelineShaderInfo, vkInfo->scenePass);
		vkInfo->device.destroyShaderModule(vsModule);
		vkInfo->device.destroyShaderModule(psModule);
	}

	void UpdateBuffers()
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();

		// Note: Alignment is done inside buffer creation
		VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

		if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
			return;
		}

		// Update buffers only if vertex or index count has been changed compared to current buffer size

		// Vertex buffer
		if ((vertexBuffer == nullptr) || (vertexCount != imDrawData->TotalVtxCount)) {
			if (vertexBuffer != nullptr) vertexBuffer->DestroyBuffer(&vkInfo->device);
			vertexBuffer = std::make_unique<Buffer<ImDrawVert>>(&vkInfo->device, imDrawData->TotalVtxCount, vk::BufferUsageFlagBits::eVertexBuffer, vkInfo->gpu.getMemoryProperties(), vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible, false);
			vertexCount = imDrawData->TotalVtxCount;
		}

		// Index buffer
		if ((indexBuffer == nullptr) || (indexCount < imDrawData->TotalIdxCount)) {
			if (indexBuffer != nullptr) indexBuffer->DestroyBuffer(&vkInfo->device);
			indexBuffer = std::make_unique<Buffer<ImDrawIdx>>(&vkInfo->device, imDrawData->TotalIdxCount, vk::BufferUsageFlagBits::eIndexBuffer, vkInfo->gpu.getMemoryProperties(), vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible, false);
			indexCount = imDrawData->TotalIdxCount;
		}

		// Upload data
		uint32_t vtxDst = 0;
		uint32_t idxDst = 0;

		for (int n = 0; n < imDrawData->CmdListsCount; n++) {
			const ImDrawList* cmdList = imDrawData->CmdLists[n];
			
			vertexBuffer->CopyData(&vkInfo->device, vtxDst, cmdList->VtxBuffer.Size, cmdList->VtxBuffer.Data);
			indexBuffer->CopyData(&vkInfo->device, idxDst, cmdList->IdxBuffer.Size, cmdList->IdxBuffer.Data);

			vtxDst += cmdList->VtxBuffer.Size;
			idxDst += cmdList->IdxBuffer.Size;
		}
	}

	void DrawFrame(vk::CommandBuffer cmd) {
		ImGuiIO& io = ImGui::GetIO();

		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descSet, 0, 0);
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

		vk::Viewport viewport(0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, 1.0f);
		cmd.setViewport(0, 1, &viewport);

		pushConstantBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		pushConstantBlock.translate = glm::vec2(-1.0f);
		cmd.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(pushConstantBlock), &pushConstantBlock);
	
		//Render commands
		ImDrawData* imDrawData = ImGui::GetDrawData();
		uint32_t vertexOffset = 0;
		uint32_t indexOffset = 0;

		if (imDrawData->CmdListsCount > 0) {

			VkDeviceSize offsets[1] = { 0 };
			cmd.bindVertexBuffers(0, 1, &vertexBuffer->GetBuffer(), offsets);
			cmd.bindIndexBuffer(indexBuffer->GetBuffer(), 0, vk::IndexType::eUint16);

			for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
			{
				const ImDrawList* cmd_list = imDrawData->CmdLists[i];
				for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
				{
					const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
					vk::Rect2D scissorRect;
					scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
					scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
					scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
					scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
					cmd.setScissor(0, 1, &scissorRect);
					cmd.drawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
					indexOffset += pcmd->ElemCount;
				}
				vertexOffset += cmd_list->VtxBuffer.Size;
			}
		}
	}
};