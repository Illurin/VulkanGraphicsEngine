#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

/*Call this function to copy pixel data from upload buffer to image*/
void Texture::SetupImage(vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp, vk::CommandPool& cmdPool, vk::Queue* queue) {
	//Create image
	auto imageInfo = vk::ImageCreateInfo()
		.setArrayLayers(isCubeMap ? 6 : 1)
		.setExtent(vk::Extent3D(width, height, 1))
		.setFormat(format)
		.setImageType(vk::ImageType::e2D)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setMipLevels(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
		.setFlags(isCubeMap ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlags())
		.setTiling(vk::ImageTiling::eOptimal);
	device->createImage(&imageInfo, 0, &image);
	
	//Allocate the memory of image
	vk::MemoryRequirements imageMemReqs;
	device->getImageMemoryRequirements(image, &imageMemReqs);

	auto imageMemoryInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(imageMemReqs.size);
	MemoryTypeFromProperties(gpuProp, imageMemReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, imageMemoryInfo.memoryTypeIndex);
	device->allocateMemory(&imageMemoryInfo, 0, &imageMemory);

	device->bindImageMemory(image, imageMemory, 0);
	
	//Set copy command
	std::vector<vk::BufferImageCopy> copyRegion;
	vk::ImageSubresourceRange subresourceRange;
	if (isCubeMap) {
		for (uint32_t i = 0; i < 6; i++) {
			auto copyRegionIndex = vk::BufferImageCopy()
				.setBufferImageHeight(0)
				.setBufferRowLength(0)
				.setBufferOffset(imageSize * i)
				.setImageOffset(vk::Offset3D(0, 0, 0))
				.setImageExtent(vk::Extent3D(width, height, 1))
				.setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, i, 1));
			copyRegion.push_back(copyRegionIndex);
		}
		subresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(6)
			.setLevelCount(1);
	}
	else {
		auto copyRegionIndex = vk::BufferImageCopy()
			.setBufferOffset(0)
			.setBufferImageHeight(0)
			.setBufferRowLength(0)
			.setImageOffset(vk::Offset3D(0, 0, 0))
			.setImageExtent(vk::Extent3D(width, height, 1))
			.setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1));
		copyRegion.push_back(copyRegionIndex);

		subresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);
	}

	vk::CommandBuffer cmd = BeginSingleTimeCommand(device, cmdPool);

	auto barrier = vk::ImageMemoryBarrier()
		.setImage(image)
		.setOldLayout(vk::ImageLayout::eUndefined)
		.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setSubresourceRange(subresourceRange)
		.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
	cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, 0, 0, 0, 1, &barrier);
	
	cmd.copyBufferToImage(uploader, image, vk::ImageLayout::eTransferDstOptimal, copyRegion.size(), copyRegion.data());

	barrier = vk::ImageMemoryBarrier()
		.setImage(image)
		.setOldLayout(vk::ImageLayout::eTransferDstOptimal)
		.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setSubresourceRange(subresourceRange)
		.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
		.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
	cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(), 0, 0, 0, 0, 1, &barrier);

	EndSingleTimeCommand(&cmd, cmdPool, device, queue);
}

void Texture::CleanUploader(vk::Device* device) {
	device->destroyBuffer(uploader);
	device->freeMemory(bufferMemory);
}

vk::ImageView Texture::GetImageView(vk::Device* device) {
	vk::ImageView imageView;

	if (isCubeMap) {
		auto texImageViewInfo = vk::ImageViewCreateInfo()
			.setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA))
			.setFormat(format)
			.setImage(image)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6))
			.setViewType(vk::ImageViewType::eCube);
		device->createImageView(&texImageViewInfo, 0, &imageView);
	}
	else {
		auto texImageViewInfo = vk::ImageViewCreateInfo()
			.setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA))
			.setFormat(format)
			.setImage(image)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.setViewType(vk::ImageViewType::e2D);
		device->createImageView(&texImageViewInfo, 0, &imageView);
	}
	
	return imageView;
}

bool LoadPixelWithWIC(const wchar_t* path, GUID tgFormat, Texture& texture, vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp) {
	IWICBitmapSource* source;

	IWICImagingFactory* factory;
	HRESULT res = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
	IWICBitmapDecoder* decoder;
	res = factory->CreateDecoderFromFilename(path, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
	
	if (FAILED(res))
		return false;

	IWICBitmapFrameDecode* frame;
	decoder->GetFrame(0, &frame);
	IWICFormatConverter* converter;
	factory->CreateFormatConverter(&converter);
	converter->Initialize(frame, tgFormat, WICBitmapDitherTypeNone, NULL, 0, WICBitmapPaletteTypeCustom);
	converter->QueryInterface(IID_PPV_ARGS(&source));
	source->GetSize(&texture.width, &texture.height);
	IWICComponentInfo* info;
	factory->CreateComponentInfo(tgFormat, &info);
	IWICPixelFormatInfo* pixelInfo;
	info->QueryInterface(IID_PPV_ARGS(&pixelInfo));
	pixelInfo->GetBitsPerPixel(&texture.BPP);

	factory->Release();
	decoder->Release();
	converter->Release();
	info->Release();
	pixelInfo->Release();

	//Calculate image information to use
	uint64_t pixelRowPitch = (uint64_t(texture.width) * uint64_t(texture.BPP) + 7) / 8;
	texture.imageSize = pixelRowPitch * (uint64_t)texture.height;

	//Create buffer
	auto bufferInfo = vk::BufferCreateInfo()
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
		.setSize(texture.imageSize);
	device->createBuffer(&bufferInfo, 0, &texture.uploader);

	//Allocate the memory of buffer
	vk::MemoryRequirements uploaderMemReqs;
	device->getBufferMemoryRequirements(texture.uploader, &uploaderMemReqs);

	auto bufferMemoryInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(uploaderMemReqs.size);
	MemoryTypeFromProperties(gpuProp, uploaderMemReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, bufferMemoryInfo.memoryTypeIndex);
	device->allocateMemory(&bufferMemoryInfo, 0, &texture.bufferMemory);

	device->bindBufferMemory(texture.uploader, texture.bufferMemory, 0);

	//Copy pixel data to the upload buffer
	BYTE* dst = nullptr;
	device->mapMemory(texture.bufferMemory, 0, uploaderMemReqs.size, vk::MemoryMapFlags(), reinterpret_cast<void**>(&dst));

	void* pixel = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, texture.imageSize);
	if (pixel == nullptr) return false;
	source->CopyPixels(nullptr, pixelRowPitch, static_cast<UINT>(texture.imageSize), reinterpret_cast<BYTE*>(pixel));

	memcpy(dst, pixel, texture.imageSize);

	device->unmapMemory(texture.bufferMemory);

	HeapFree(GetProcessHeap(), 0, pixel);

	return true;
}

bool LoadCubeMapWithWIC(const wchar_t* path, GUID tgFormat, Texture& texture, vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp) {
	IWICBitmapSource* source;

	IWICImagingFactory* factory;
	HRESULT res = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
	IWICBitmapDecoder* decoder;
	res = factory->CreateDecoderFromFilename(path, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);

	if (FAILED(res))
		return false;

	IWICBitmapFrameDecode* frame;
	decoder->GetFrame(0, &frame);
	IWICFormatConverter* converter;
	factory->CreateFormatConverter(&converter);
	converter->Initialize(frame, tgFormat, WICBitmapDitherTypeNone, NULL, 0, WICBitmapPaletteTypeCustom);
	converter->QueryInterface(IID_PPV_ARGS(&source));
	source->GetSize(&texture.width, &texture.height);
	IWICComponentInfo* info;
	factory->CreateComponentInfo(tgFormat, &info);
	IWICPixelFormatInfo* pixelInfo;
	info->QueryInterface(IID_PPV_ARGS(&pixelInfo));
	pixelInfo->GetBitsPerPixel(&texture.BPP);

	factory->Release();
	decoder->Release();
	converter->Release();
	info->Release();
	pixelInfo->Release();
	
	texture.isCubeMap = true;

	//计算出立方体贴图的每个面信息
	uint64_t x = static_cast<uint64_t>(texture.width / 4);
	uint64_t y = static_cast<uint64_t>(texture.height / 3);

	texture.width = static_cast<uint32_t>(x);
	texture.height = static_cast<uint32_t>(y);

	uint64_t offsetX[6] = { 2 * x, 0, x, x, x, 3 * x };
	uint64_t offsetY[6] = { y, y, 0, 2 * y , y, y };

	uint64_t pixelRowPitch = (x * (uint64_t)texture.BPP + 7) / 8;
	texture.imageSize = y * pixelRowPitch;
	uint64_t bufferSize = texture.imageSize * 6;

	//创建用作上传的缓存
	auto bufferInfo = vk::BufferCreateInfo()
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
		.setSize(bufferSize);
	device->createBuffer(&bufferInfo, 0, &texture.uploader);

	//Allocate the memory of buffer
	vk::MemoryRequirements uploaderMemReqs;
	device->getBufferMemoryRequirements(texture.uploader, &uploaderMemReqs);

	auto bufferMemoryInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(uploaderMemReqs.size);
	MemoryTypeFromProperties(gpuProp, uploaderMemReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, bufferMemoryInfo.memoryTypeIndex);
	device->allocateMemory(&bufferMemoryInfo, 0, &texture.bufferMemory);

	device->bindBufferMemory(texture.uploader, texture.bufferMemory, 0);

	//Copy pixel data to the upload buffer
	BYTE* dst = nullptr;
	device->mapMemory(texture.bufferMemory, 0, uploaderMemReqs.size, vk::MemoryMapFlags(), reinterpret_cast<void**>(&dst));

	for (uint32_t i = 0; i < 6; i++) {
		void* pixel = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, texture.imageSize);
		if (pixel == nullptr) return false;

		WICRect scissor = { offsetX[i], offsetY[i], x, y };
		source->CopyPixels(&scissor, pixelRowPitch, static_cast<uint32_t>(texture.imageSize), reinterpret_cast<BYTE*>(pixel));

		memcpy(dst + texture.imageSize * i, pixel, texture.imageSize);

		HeapFree(GetProcessHeap(), 0, pixel);
	}
	device->unmapMemory(texture.bufferMemory);

	return true;
}

void LoadPixelWithSTB(const char* path, uint32_t BPP, Texture& texture, vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp) {
	int channelInFile;
	stbi_uc* source = stbi_load(path, reinterpret_cast<int*>(&texture.width), reinterpret_cast<int*>(&texture.height), &channelInFile, 4);

	texture.BPP = BPP;

	//Calculate image information to use
	uint64_t pixelRowPitch = (uint64_t(texture.width) * uint64_t(texture.BPP) + 7) / 8;
	texture.imageSize = pixelRowPitch * (uint64_t)texture.height;

	//Create buffer
	auto bufferInfo = vk::BufferCreateInfo()
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
		.setSize(texture.imageSize);
	device->createBuffer(&bufferInfo, 0, &texture.uploader);

	//Allocate the memory of buffer
	vk::MemoryRequirements uploaderMemReqs;
	device->getBufferMemoryRequirements(texture.uploader, &uploaderMemReqs);

	auto bufferMemoryInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(uploaderMemReqs.size);
	MemoryTypeFromProperties(gpuProp, uploaderMemReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, bufferMemoryInfo.memoryTypeIndex);
	device->allocateMemory(&bufferMemoryInfo, 0, &texture.bufferMemory);

	device->bindBufferMemory(texture.uploader, texture.bufferMemory, 0);

	//Copy pixel data to the upload buffer
	BYTE* dst = nullptr;
	device->mapMemory(texture.bufferMemory, 0, uploaderMemReqs.size, vk::MemoryMapFlags(), reinterpret_cast<void**>(&dst));

	memcpy(dst, source, texture.imageSize);

	device->unmapMemory(texture.bufferMemory);

	stbi_image_free(source);
}