#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define NUM_BONES_PER_VERTEX 4
#define MAX_BONE_NUM 500

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_win32.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include "PlayerController.h"

#include <unordered_map>
#include <map>
#include <fstream>

vk::ShaderModule CreateShaderModule(const std::string& path, vk::Device device);
vk::Pipeline CreateGraphicsPipeline(vk::Device, vk::PipelineDynamicStateCreateInfo, vk::PipelineVertexInputStateCreateInfo, vk::PipelineInputAssemblyStateCreateInfo, vk::PipelineRasterizationStateCreateInfo, vk::PipelineColorBlendStateCreateInfo, vk::PipelineViewportStateCreateInfo, vk::PipelineDepthStencilStateCreateInfo, vk::PipelineMultisampleStateCreateInfo, vk::PipelineLayout, std::vector<vk::PipelineShaderStageCreateInfo>&, vk::RenderPass);

bool MemoryTypeFromProperties(vk::PhysicalDeviceMemoryProperties memProp, uint32_t typeBits, vk::MemoryPropertyFlags requirementMask, uint32_t& typeIndex);
vk::CommandBuffer BeginSingleTimeCommand(vk::Device* device, const vk::CommandPool& cmdPool);
void EndSingleTimeCommand(vk::CommandBuffer* cmd, vk::CommandPool& cmdPool, vk::Device* device, vk::Queue* queue);

/*Constant buffer class*/
template<typename T>
class Buffer {
public:
	Buffer(vk::Device* device, uint32_t elementCount, vk::BufferUsageFlags usage, vk::PhysicalDeviceMemoryProperties gpuProp, vk::MemoryPropertyFlags memProp, bool mapped) {
		elementByteSize = sizeof(T);
		this->mapped = mapped;

		auto bufferInfo = vk::BufferCreateInfo()
			.setSize(elementByteSize * (uint64_t)elementCount)
			.setUsage(usage);

		device->createBuffer(&bufferInfo, 0, &buffer);

		vk::MemoryRequirements memReqs;
		device->getBufferMemoryRequirements(buffer, &memReqs);

		auto memoryInfo = vk::MemoryAllocateInfo()
			.setAllocationSize(memReqs.size);

		MemoryTypeFromProperties(gpuProp, memReqs.memoryTypeBits, memProp, memoryInfo.memoryTypeIndex);

		device->allocateMemory(&memoryInfo, 0, &memory);

		device->bindBufferMemory(buffer, memory, 0);

		if (mapped)
			device->mapMemory(memory, 0, elementByteSize * (uint64_t)elementCount, vk::MemoryMapFlags(), reinterpret_cast<void**>(&mappedData));
	}

	void DestroyBuffer(vk::Device* device) {
		if (mapped)
			device->unmapMemory(memory);

		device->destroyBuffer(buffer, 0);
		device->freeMemory(memory, 0);
	}

	void CopyData(vk::Device* device, uint32_t elementIndex, uint32_t elementCount, const T* data) {
		if (!mapped)
			device->mapMemory(memory, 0, elementByteSize * (uint64_t)elementCount, vk::MemoryMapFlags(), reinterpret_cast<void**>(&mappedData));

		memcpy(&mappedData[elementIndex * elementByteSize], data, elementByteSize * elementCount);

		if (!mapped)
			device->unmapMemory(memory);
	}

	vk::Buffer GetBuffer()const {
		return buffer;
	}

private:
	vk::Buffer buffer;
	vk::DeviceMemory memory;

	bool mapped = false;

	BYTE* mappedData = nullptr;
	uint64_t elementByteSize = 0;
};

/*===========================================数据结构===========================================*/
struct Vertex {
	glm::vec3 position;
	glm::vec2 texCoord;
	glm::vec3 normal;
};

struct SkinnedVertex {
	glm::vec3 position;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec3 boneWeights;
	uint32_t boneIndices[4];
};

struct Light {
	glm::vec3 strength = glm::vec3(0.0f);
	float fallOffStart;					   //point/spot light only
	glm::vec3 direction;					   //directional/spot light only
	float fallOffEnd;					   //point/spot light only
	glm::vec3 position;					   //point/spot light only
	float spotPower;					   //spot light only
};

struct PassConstants {
	glm::mat4x4 viewMatrix;
	glm::mat4x4 projMatrix;
	glm::mat4x4 shadowTransform;

	glm::vec4 eyePos;
	glm::vec4 ambientLight;

	Light lights[3];
};

struct ObjectConstants {
	glm::mat4x4 worldMatrix;
};

struct MaterialConstants {
	glm::mat4x4 matTransform;
	glm::vec4 diffuseAlbedo;
	glm::vec3 fresnelR0;
	float roughness;
};

struct SkinnedConstants {
	glm::mat4x4 boneTransforms[MAX_BONE_NUM];
};

/*=============================================================================================*/

/*Base Vulkan struct for storing*/
struct Vulkan {
	vk::Format format;

	vk::Instance instance;
	vk::PhysicalDevice gpu;
	vk::Device device;
	vk::Queue queue;
	vk::SurfaceKHR surface;
	vk::SwapchainKHR swapchain;
	vk::CommandPool cmdPool;
	vk::CommandBuffer cmd;

	vk::Semaphore imageAcquiredSemaphore;
	vk::Fence fence;

#ifndef NDEBUG
	vk::DebugUtilsMessengerEXT debugMessenger;
#endif

	struct {
		vk::VertexInputBindingDescription binding;
		std::vector<vk::VertexInputAttributeDescription> attrib;
	}vertex;

	struct {
		vk::Format format;
		vk::Image image;
		vk::DeviceMemory memory;
		vk::ImageView imageView;
	}depth;

	struct {
		vk::Image image;
		vk::DeviceMemory memory;
		vk::ImageView imageView;
		vk::Framebuffer framebuffer;
	}scene;

	std::unordered_map<std::string, vk::Pipeline> pipelines;
	std::unordered_map<std::string, vk::PipelineLayout> pipelineLayout;

	vk::RenderPass scenePass;
	vk::RenderPass finalPass;

	vk::DescriptorPool descPool;
	std::vector<vk::DescriptorSetLayout> descSetLayout;

	vk::DescriptorSetLayout finalPassLayout;
	std::vector<vk::DescriptorSet> finalPassDescSets;

	std::vector<vk::Framebuffer> finalFramebuffers;

	std::vector<const char*> instanceExtensions;
	std::vector<const char*> deviceExtensions;
	std::vector<const char*> validationLayers;
	std::vector<vk::QueueFamilyProperties> queueProp;
	std::vector<vk::Image> swapchainImages;
	std::vector<vk::ImageView> swapchainImageViews;

	uint32_t width, height;
	uint32_t graphicsQueueFamilyIndex;
	uint32_t frameCount;

	PlayerInput input;
};