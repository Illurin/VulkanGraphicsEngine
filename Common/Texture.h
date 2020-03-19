#pragma once
#include <wincodec.h>
#include "vkUtil.h"

class Texture {
public:
	vk::Format format = vk::Format::eR8G8B8A8Unorm;

	void SetupImage(vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp, vk::CommandPool& cmdPool, vk::Queue* queue);
	void CleanUploader(vk::Device* device);

	vk::ImageView GetImageView(vk::Device* device);
	vk::Image GetImage() {
		return image;
	}

	UINT width, height, BPP;
	uint64_t imageSize;

	vk::Buffer uploader;
	vk::DeviceMemory bufferMemory;

	bool isCubeMap = false;

private:
	vk::Image image;
	vk::DeviceMemory imageMemory;
};

bool LoadPixelWithWIC(const wchar_t* path, GUID tgFormat, Texture& texture, vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp);
bool LoadCubeMapWithWIC(const wchar_t* path, GUID tgFormat, Texture& texture, vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp);
void LoadPixelWithSTB(const char* path, uint32_t BPP, Texture& texture, vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp);