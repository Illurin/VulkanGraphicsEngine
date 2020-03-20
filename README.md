# VulkanGraphicsEngine
基于Vulkan的封装引擎框架

封装了：渲染项 + 光照 + 阴影 + 导入模型 + 骨骼动画 + 基于CPU的火焰烟雾粒子效果 + bloom特效

目前正在修改封装框架，提交在engine分支中

一个Windows Vulkan程序需要的所有代码：
```
#define VK_USE_PLATFORM_WIN32_KHR

#include "framework.h"
#include "VulkanGraphicsEngine.h"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_win32.h"
#include "glm/glm.hpp"

#include <fstream>

struct Vertex {
	glm::vec3 position;
	glm::vec4 color;
};

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
	vk::RenderPass renderPass;
	vk::PipelineCache pipelineCache;
	vk::PipelineLayout pipelineLayout;
	vk::Pipeline pipeline;

	vk::Semaphore imageAcquiredSemaphore;
	vk::Fence fence;

	vk::DebugUtilsMessengerEXT debugMessenger;

	struct {
		vk::Buffer buffer;
		vk::DeviceMemory memory;
		vk::VertexInputBindingDescription binding;
		std::vector<vk::VertexInputAttributeDescription> attrib;
	}vertex;
	
	std::vector<const char*> instanceExtensions;
	std::vector<const char*> deviceExtensions;
	std::vector<const char*> validationLayers;
	std::vector<vk::QueueFamilyProperties> queueProp;
	std::vector<vk::Image> swapchainImages;
	std::vector<vk::ImageView> swapchainImageViews;
	std::vector<vk::Framebuffer> framebuffers;
	
	uint32_t width, height;
	uint32_t graphicsQueueFamilyIndex;
	uint32_t frameCount;
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	vk::DebugUtilsMessageTypeFlagsEXT messageType,
	const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	OutputDebugStringA(pCallbackData->pMessage);
	return VK_FALSE;
}

bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers) {
	uint32_t layerCount = 0;
	vk::enumerateInstanceLayerProperties(&layerCount, static_cast<vk::LayerProperties*>(nullptr));
	if (layerCount == 0)
		return false;
	std::vector<vk::LayerProperties> availableLayers(layerCount);
	vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}
	return true;
}

bool MemoryTypeFromProperties(vk::PhysicalDeviceMemoryProperties memProp, uint32_t typeBits, vk::MemoryPropertyFlags requirementMask, uint32_t& typeIndex) {
	for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
		if ((typeBits & 1) == 1) {
			if ((memProp.memoryTypes[i].propertyFlags & requirementMask) == requirementMask) {
				typeIndex = i;
				return true;
			}
		}
	}
	return false;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	//Initialize
	int windowWidth = 1024;
	int windowHeight = 760;

	MSG msg = {};
	HWND hWnd = nullptr;

	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_GLOBALCLASS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszClassName = L"MainWindowClass";
	RegisterClassEx(&wcex);

	hWnd = CreateWindowW(L"MainWindowClass", L"Base 3D Project", WS_OVERLAPPED | WS_SYSMENU, CW_USEDEFAULT, 0, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	/*1. Initialize*/
	Vulkan vkInfo;
	vkInfo.instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	vkInfo.instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	vkInfo.deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	vkInfo.width = windowWidth;
	vkInfo.height = windowHeight;

	/*Set validation layer*/
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	if (enableValidationLayers && !CheckValidationLayerSupport(validationLayers)) {
		MessageBox(0, L"Dont support validation layers!!!", 0, 0);
	}

	if (enableValidationLayers) {
		vkInfo.validationLayers = validationLayers;
		vkInfo.instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	
	vk::DynamicLoader dl;
	PFN_vkGetInstanceProcAddr GetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	PFN_vkDebugUtilsMessengerCallbackEXT CallbackFunc = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(DebugCallback);
	
	auto debugMessengerInfo = vk::DebugUtilsMessengerCreateInfoEXT()
		.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
		.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
		.setPfnUserCallback(CallbackFunc)
		.setPUserData(nullptr);

	/*2. Create instance*/

	//2.1 Application info
	vk::ApplicationInfo applicationInfo;
	applicationInfo.setApiVersion(VK_API_VERSION_1_0);
	applicationInfo.setPEngineName("Vulkan Graphics Engine");
	applicationInfo.setEngineVersion(1);
	applicationInfo.setPApplicationName("Base Project");
	applicationInfo.setApplicationVersion(1);

	//2.2 Create instance
	vk::InstanceCreateInfo instanceInfo;
	instanceInfo.setEnabledExtensionCount(vkInfo.instanceExtensions.size());
	instanceInfo.setPpEnabledExtensionNames(vkInfo.instanceExtensions.data());
	instanceInfo.setPApplicationInfo(&applicationInfo);
	instanceInfo.setEnabledLayerCount(vkInfo.validationLayers.size());
	instanceInfo.setPpEnabledLayerNames(vkInfo.validationLayers.data());
	
	if (vk::createInstance(&instanceInfo, 0, &vkInfo.instance) != vk::Result::eSuccess) {
		MessageBox(0, L"Create instance failed!!!", 0, 0);
	}
	
	//2.3 Create debug messengr
	vk::DispatchLoaderDynamic dispatch(vkInfo.instance, GetInstanceProcAddr);
	if (vkInfo.instance.createDebugUtilsMessengerEXT(&debugMessengerInfo, 0, &vkInfo.debugMessenger, dispatch) != vk::Result::eSuccess) {
		MessageBox(0, L"Create debug messenger failed!!!", 0, 0);
	}

	/*3. Enumerate physical device*/
	uint32_t gpuCount = 0;
	if (vkInfo.instance.enumeratePhysicalDevices(&gpuCount, static_cast<vk::PhysicalDevice*>(nullptr)) != vk::Result::eSuccess) {
		MessageBox(0, L"Cannot enumerate physical devices!!!", 0, 0);
	}

	if (gpuCount > 0) {
		std::vector<vk::PhysicalDevice> gpus(gpuCount);
		vkInfo.instance.enumeratePhysicalDevices(&gpuCount, gpus.data());
		vkInfo.gpu = gpus[0];
	}
	else {
		MessageBox(0, L"The number of GPU is 0!!!", 0, 0);
	}

	/*4. Create device*/
	vk::DeviceCreateInfo deviceInfo;
	vk::DeviceQueueCreateInfo deviceQueueInfo;

	//4.1 Enumerate queue family properties
	uint32_t queueFamilyCount = 0;

	vkInfo.gpu.getQueueFamilyProperties(&queueFamilyCount, static_cast<vk::QueueFamilyProperties*>(nullptr));
	vkInfo.queueProp.resize(queueFamilyCount);
	vkInfo.gpu.getQueueFamilyProperties(&queueFamilyCount, vkInfo.queueProp.data());

	bool found = false;
	for (size_t i = 0; i < queueFamilyCount; i++) {
		if (vkInfo.queueProp[i].queueFlags & vk::QueueFlagBits::eGraphics) {
			vkInfo.graphicsQueueFamilyIndex = i;
			found = true;
			break;
		}
	}
	if(!found)
		MessageBox(0, L"Cannot find graphics queue!!!", 0, 0);

	//4.2 Create device
	float priorities[1] = { 0.0f };
	deviceQueueInfo.setQueueCount(1);
	deviceQueueInfo.setPQueuePriorities(priorities);
	deviceQueueInfo.setQueueFamilyIndex(vkInfo.graphicsQueueFamilyIndex);
	deviceInfo.setQueueCreateInfoCount(1);
	deviceInfo.setPQueueCreateInfos(&deviceQueueInfo);
	deviceInfo.setEnabledExtensionCount(vkInfo.deviceExtensions.size());
	deviceInfo.setPpEnabledExtensionNames(vkInfo.deviceExtensions.data());

	if (vkInfo.gpu.createDevice(&deviceInfo, 0, &vkInfo.device) != vk::Result::eSuccess) {
		MessageBox(0, L"Create device failed!!!", 0, 0);
	}

	/*5. Create command buffers*/

	//5.1 Create command pool
	vk::CommandPoolCreateInfo commandPoolInfo;
	commandPoolInfo.setQueueFamilyIndex(vkInfo.graphicsQueueFamilyIndex);
	commandPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
	if (vkInfo.device.createCommandPool(&commandPoolInfo, 0, &vkInfo.cmdPool) != vk::Result::eSuccess) {
		MessageBox(0, L"Create command pool failed!!!", 0, 0);
	}

	//5.2 Allocate Command buffer from the pool
	vk::CommandBufferAllocateInfo cmdBufferAlloc;
	cmdBufferAlloc.setCommandPool(vkInfo.cmdPool);
	cmdBufferAlloc.setLevel(vk::CommandBufferLevel::ePrimary);
	cmdBufferAlloc.setCommandBufferCount(1);
	if (vkInfo.device.allocateCommandBuffers(&cmdBufferAlloc, &vkInfo.cmd) != vk::Result::eSuccess) {
		MessageBox(0, L"Allocate command buffer failed!!!", 0, 0);
	}

	/*6. Create a swap chain*/

	//6.1 Create a surface fo win32
	const auto surfaceInfo = vk::Win32SurfaceCreateInfoKHR()
		.setHwnd(hWnd)
		.setHinstance(hInstance);
	if (vkInfo.instance.createWin32SurfaceKHR(&surfaceInfo, 0, &vkInfo.surface) != vk::Result::eSuccess) {
		MessageBox(0, L"Create Win32 surface failed!!!", 0, 0);
	}

	//6.2 Find present queue family
	std::vector<vk::Bool32> supportPresents(queueFamilyCount);
	for (size_t i = 0; i < queueFamilyCount; i++)
		vkInfo.gpu.getSurfaceSupportKHR(i, vkInfo.surface, &supportPresents[i]);

	found = false;
	for (size_t i = 0; i < queueFamilyCount; i++) {
		if (vkInfo.queueProp[i].queueFlags & vk::QueueFlagBits::eGraphics) {
			if (supportPresents[i] == VK_TRUE) {
				vkInfo.graphicsQueueFamilyIndex = i;
				found = true;
				break;
			}
		}
	}
	if(!found)
		MessageBox(0, L"Do not support both graphics and present family!!!", 0, 0);

	//6.3 Create swap chain
	uint32_t formatCount = 0;
	vkInfo.gpu.getSurfaceFormatsKHR(vkInfo.surface, &formatCount, static_cast<vk::SurfaceFormatKHR*>(nullptr));
	if (formatCount == 0) {
		MessageBox(0, L"Cannot get surface formats!!!", 0, 0);
	}
	std::vector<vk::SurfaceFormatKHR> surfaceFormats(formatCount);
	vkInfo.gpu.getSurfaceFormatsKHR(vkInfo.surface, &formatCount, surfaceFormats.data());

	uint32_t presentModeCount = 0;
	vkInfo.gpu.getSurfacePresentModesKHR(vkInfo.surface, &presentModeCount, static_cast<vk::PresentModeKHR*>(nullptr));
	if (presentModeCount == 0) {
		MessageBox(0, L"Cannot get surface present modes!!!", 0, 0);
	}
	std::vector<vk::PresentModeKHR> presentModes(presentModeCount);
	vkInfo.gpu.getSurfacePresentModesKHR(vkInfo.surface, &presentModeCount, presentModes.data());

	vk::SurfaceCapabilitiesKHR surfaceCapabilities;
	vkInfo.gpu.getSurfaceCapabilitiesKHR(vkInfo.surface, &surfaceCapabilities);
	
	vkInfo.frameCount = surfaceCapabilities.minImageCount;
	vkInfo.format = vk::Format::eR8G8B8A8Unorm;

	auto swapchainInfo = vk::SwapchainCreateInfoKHR()
		.setSurface(vkInfo.surface)
		.setImageFormat(vkInfo.format)
		.setMinImageCount(vkInfo.frameCount)
		.setImageExtent(vk::Extent2D(vkInfo.width, vkInfo.height))
		.setPreTransform(surfaceCapabilities.currentTransform)
		.setPresentMode(vk::PresentModeKHR::eFifo)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setImageColorSpace(surfaceFormats[0].colorSpace)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setImageArrayLayers(1)
		.setClipped(true);
	
	if (vkInfo.device.createSwapchainKHR(&swapchainInfo, 0, &vkInfo.swapchain) != vk::Result::eSuccess) {
		MessageBox(0, L"Create swap chain failed!!!", 0, 0);
	}

	//6.4 Create frame buffer image views
	vkInfo.device.getSwapchainImagesKHR(vkInfo.swapchain, &vkInfo.frameCount, static_cast<vk::Image*>(nullptr));
	if (vkInfo.frameCount > 1) {
		vkInfo.swapchainImages.resize(vkInfo.frameCount);
		vkInfo.device.getSwapchainImagesKHR(vkInfo.swapchain, &vkInfo.frameCount, vkInfo.swapchainImages.data());
	}
	else {
		MessageBox(0, L"Swap chain images error!!!", 0, 0);
	}

	vkInfo.swapchainImageViews.resize(vkInfo.frameCount);

	for (size_t i = 0; i < vkInfo.frameCount; i++) {
		auto createInfo = vk::ImageViewCreateInfo()
			.setViewType(vk::ImageViewType::e2D)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.setFormat(vkInfo.format)
			.setImage(vkInfo.swapchainImages[i])
			.setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity));
		if (vkInfo.device.createImageView(&createInfo, 0, &vkInfo.swapchainImageViews[i]) != vk::Result::eSuccess) {
			MessageBox(0, L"Create swap chain image views failed!!!", 0, 0);
		}
	}

	//6.5 Get queue
	vkInfo.device.getQueue(vkInfo.graphicsQueueFamilyIndex, 0, &vkInfo.queue);

	/*11. Create render pass*/

	//11.1 Prepare attachments
	auto attachment = vk::AttachmentDescription()
		.setFormat(vkInfo.format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	//11.2 Prepare subpass
	auto colorReference = vk::AttachmentReference()
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	auto subpass = vk::SubpassDescription()
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorReference);

	//11.3 Create render pass
	auto renderPassInfo = vk::RenderPassCreateInfo()
		.setAttachmentCount(1)
		.setPAttachments(&attachment)
		.setSubpassCount(1)
		.setPSubpasses(&subpass);

	if (vkInfo.device.createRenderPass(&renderPassInfo, 0, &vkInfo.renderPass) != vk::Result::eSuccess) {
		MessageBox(0, L"Create render pass failed!!!", 0, 0);
	}

	/*12. Compile shader module*/

	//12.1 Create shader module
	auto loadShaderByteCode = [](const std::string& fileName) {
		std::ifstream loadFile(fileName, std::ios::ate | std::ios::binary);
		if (!loadFile.is_open()) {
			MessageBox(0, L"Cannot open the shader file!!!", 0, 0);
		}
		size_t fileSize = (size_t)loadFile.tellg();
		std::vector<char> buffer(fileSize);
		loadFile.seekg(0);
		loadFile.read(buffer.data(), fileSize);
		loadFile.close();
		return buffer;
	};

	auto createShaderModule = [](vk::Device device, const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
			MessageBox(0, L"Create shader module failed!!!", 0, 0);
		}

		return shaderModule;
	};

	auto vsCode = loadShaderByteCode("Shaders\\vertex.spv");
	auto psCode = loadShaderByteCode("Shaders\\fragment.spv");

	auto vsModule = createShaderModule(vkInfo.device, vsCode);
	auto psModule = createShaderModule(vkInfo.device, psCode);

	//12.2 Create pipeline shader module
	vk::PipelineShaderStageCreateInfo pipelineShaderInfo[2];

	pipelineShaderInfo[0] = vk::Pipeline
