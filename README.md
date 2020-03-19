# VulkanGraphicsEngine
基于Vulkan的封装引擎框架

一个Windows Vulkan程序需要的所有代码：

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

	pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(vsModule)
		.setStage(vk::ShaderStageFlagBits::eVertex);

	pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(psModule)
		.setStage(vk::ShaderStageFlagBits::eFragment);

	/*13. Create frame buffers*/
	auto frameImageView = vk::ImageView();
	auto framebufferInfo = vk::FramebufferCreateInfo()
		.setRenderPass(vkInfo.renderPass)
		.setAttachmentCount(1)
		.setPAttachments(&frameImageView)
		.setWidth(vkInfo.width)
		.setHeight(vkInfo.height)
		.setLayers(1);

	vkInfo.framebuffers.resize(vkInfo.frameCount);

	for (size_t i = 0; i < vkInfo.frameCount; i++) {
		frameImageView = vkInfo.swapchainImageViews[i];
		if (vkInfo.device.createFramebuffer(&framebufferInfo, 0, &vkInfo.framebuffers[i]) != vk::Result::eSuccess) {
			MessageBox(0, L"Create frame buffer failed!!!", 0, 0);
		}
	}

	/*Create vertex buffer & index buffer*/
	std::vector<Vertex> vertices(3);

	vertices[0].position = glm::vec3(0.0f, 0.5f, 0.0f);
	vertices[1].position = glm::vec3(-0.5f, -0.5f, 0.0f);
	vertices[2].position = glm::vec3(0.5f, -0.5f, 0.0f);
	vertices[0].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	vertices[1].color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	vertices[2].color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

	auto vertexBufferInfo = vk::BufferCreateInfo()
		.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
		.setSize(sizeof(Vertex) * vertices.size());

	vkInfo.device.createBuffer(&vertexBufferInfo, 0, &vkInfo.vertex.buffer);

	vk::MemoryRequirements memReqs;
	vkInfo.device.getBufferMemoryRequirements(vkInfo.vertex.buffer, &memReqs);

	auto allocInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(memReqs.size);
	MemoryTypeFromProperties(vkInfo.gpu.getMemoryProperties(), memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, allocInfo.memoryTypeIndex);

	vkInfo.device.allocateMemory(&allocInfo, 0, &vkInfo.vertex.memory);

	uint8_t* ptr;
	vkInfo.device.mapMemory(vkInfo.vertex.memory, 0, memReqs.size, vk::MemoryMapFlags(), reinterpret_cast<void**>(&ptr));

	memcpy(ptr, vertices.data(), sizeof(Vertex) * vertices.size());

	vkInfo.device.unmapMemory(vkInfo.vertex.memory);

	vkInfo.device.bindBufferMemory(vkInfo.vertex.buffer, vkInfo.vertex.memory, 0);

	vkInfo.vertex.binding.setBinding(0);
	vkInfo.vertex.binding.setInputRate(vk::VertexInputRate::eVertex);
	vkInfo.vertex.binding.setStride(sizeof(Vertex));

	vkInfo.vertex.attrib.resize(2);

	vkInfo.vertex.attrib[0].setBinding(0);
	vkInfo.vertex.attrib[0].setFormat(vk::Format::eR32G32B32Sfloat);
	vkInfo.vertex.attrib[0].setLocation(0);
	vkInfo.vertex.attrib[0].setOffset(0);

	vkInfo.vertex.attrib[1].setBinding(0);
	vkInfo.vertex.attrib[1].setFormat(vk::Format::eR32G32B32A32Sfloat);
	vkInfo.vertex.attrib[1].setLocation(1);
	vkInfo.vertex.attrib[1].setOffset(sizeof(glm::vec3));

	/*14. Create a graphics pipeline state*/

	//14.1 Dynamic state
	auto dynamicInfo = vk::PipelineDynamicStateCreateInfo();
	std::vector<vk::DynamicState> dynamicStates;

	//14.2 Vertex input state
	auto viInfo = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptionCount(1)
		.setPVertexBindingDescriptions(&vkInfo.vertex.binding)
		.setVertexAttributeDescriptionCount(2)
		.setPVertexAttributeDescriptions(vkInfo.vertex.attrib.data());

	//14.3 Input assembly state
	auto iaInfo = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(vk::PrimitiveTopology::eTriangleList)
		.setPrimitiveRestartEnable(VK_FALSE);

	//14.4 Rasterization state
	auto rsInfo = vk::PipelineRasterizationStateCreateInfo()
		.setCullMode(vk::CullModeFlagBits::eNone)
		.setDepthBiasEnable(VK_FALSE)
		.setDepthClampEnable(VK_FALSE)
		.setFrontFace(vk::FrontFace::eClockwise)
		.setLineWidth(1.0f)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setRasterizerDiscardEnable(VK_FALSE);

	//14.5 Color blend state
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

	//14.6 Viewport state
	vk::Viewport viewport;
	viewport.setMaxDepth(1.0f);
	viewport.setMinDepth(0.0f);
	viewport.setX(0.0f);
	viewport.setY(0.0f);
	viewport.setWidth(vkInfo.width);
	viewport.setHeight(vkInfo.height);

	auto scissor = vk::Rect2D()
		.setOffset(vk::Offset2D(0.0f, 0.0f))
		.setExtent(vk::Extent2D(vkInfo.width, vkInfo.height));

	auto vpInfo = vk::PipelineViewportStateCreateInfo()
		.setScissorCount(1)
		.setPScissors(&scissor)
		.setViewportCount(1)
		.setPViewports(&viewport);

	//14.7 Depth stencil state
	auto dsInfo = vk::PipelineDepthStencilStateCreateInfo()
		.setDepthTestEnable(VK_FALSE)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setStencilTestEnable(VK_FALSE);

	//14.8 Multisample state
	auto msInfo = vk::PipelineMultisampleStateCreateInfo()
		.setAlphaToCoverageEnable(VK_FALSE)
		.setAlphaToOneEnable(VK_FALSE)
		.setMinSampleShading(0.0f)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		.setSampleShadingEnable(VK_FALSE);

	//14.9 Pipeline layout
	auto plInfo = vk::PipelineLayoutCreateInfo()
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(0)
		.setSetLayoutCount(0)
		.setPSetLayouts(0);
	if (vkInfo.device.createPipelineLayout(&plInfo, 0, &vkInfo.pipelineLayout) != vk::Result::eSuccess) {
		MessageBox(0, L"Create pipeline layout failed!!!", 0, 0);
	}

	//14.10 Create pipeline cache
	const vk::PipelineCacheCreateInfo pipelineCacheInfo;
	if (vkInfo.device.createPipelineCache(&pipelineCacheInfo, 0, &vkInfo.pipelineCache) != vk::Result::eSuccess) {
		MessageBox(0, L"Create pipeline cache failed!!!", 0, 0);
	}

	//14.11 Create pipeline state
	auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
		.setLayout(vkInfo.pipelineLayout)
		.setPColorBlendState(&cbInfo)
		.setPDepthStencilState(&dsInfo)
		.setPDynamicState(&dynamicInfo)
		.setPMultisampleState(&msInfo)
		.setPRasterizationState(&rsInfo)
		.setStageCount(2)
		.setPStages(pipelineShaderInfo)
		.setPViewportState(&vpInfo)
		.setRenderPass(vkInfo.renderPass)
		.setPInputAssemblyState(&iaInfo)
		.setPVertexInputState(&viInfo);

	if (vkInfo.device.createGraphicsPipelines(vkInfo.pipelineCache, 1, &pipelineInfo, nullptr, &vkInfo.pipeline) != vk::Result::eSuccess) {
		MessageBox(0, L"Create graphics pipeline failed!!!", 0, 0);
	}

	/*15. Draw the geometry*/

	//15.1 Prepare semaphore and fence
	auto semaphoreInfo = vk::SemaphoreCreateInfo();
	if (vkInfo.device.createSemaphore(&semaphoreInfo, 0, &vkInfo.imageAcquiredSemaphore) != vk::Result::eSuccess) {
		MessageBox(0, L"Create image acquired semaphore failed!!!", 0, 0);
	}
	
	auto fenceInfo = vk::FenceCreateInfo();
	if (vkInfo.device.createFence(&fenceInfo, 0, &vkInfo.fence) != vk::Result::eSuccess) {
		MessageBox(0, L"Create fence failed!!!", 0, 0);
	}

	//15.2 Prepare render pass begin info
	vk::ClearValue clearValue[1] = {
		vk::ClearColorValue(std::array<float, 4>({0.2f, 0.2f, 0.2f, 0.2f}))
	};
	
	auto renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setRenderPass(vkInfo.renderPass)
		.setRenderArea(vk::Rect2D(vk::Offset2D(0.0f, 0.0f), vk::Extent2D(vkInfo.width, vkInfo.height)))
		.setClearValueCount(1)
		.setPClearValues(clearValue);

	//15.3 Begin record commands
	auto cmdBeginInfo = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	//Update
	HANDLE phWait = CreateWaitableTimer(NULL, FALSE, NULL);
	LARGE_INTEGER liDueTime = {};
	liDueTime.QuadPart = -1i64;
	SetWaitableTimer(phWait, &liDueTime, 30.0f, NULL, NULL, 0);

	DWORD dwRet = 0;
	BOOL bExit = FALSE;
	while (!bExit)
	{
		dwRet = MsgWaitForMultipleObjects(1, &phWait, FALSE, INFINITE, QS_ALLINPUT);
		switch (dwRet - WAIT_OBJECT_0)
		{
		case 0:
		case WAIT_TIMEOUT:
		{
			//15.4 Begin record commands
			if (vkInfo.cmd.begin(&cmdBeginInfo) != vk::Result::eSuccess) {
				MessageBox(0, L"Begin record command failed!!!", 0, 0);
			}

			//15.5 Wait for swap chain
			uint32_t currentBuffer = 0;
			if (vkInfo.device.acquireNextImageKHR(vkInfo.swapchain, UINT64_MAX, vkInfo.imageAcquiredSemaphore, vk::Fence(), &currentBuffer) != vk::Result::eSuccess) {
				MessageBox(0, L"Acquire next image failed!!!", 0, 0);
			}

			//15.6 Begin render pass
			renderPassBeginInfo.setFramebuffer(vkInfo.framebuffers[currentBuffer]);
			vkInfo.cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
			
			//15.7 Bind Pipeline state
			vkInfo.cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo.pipeline);
			
			//15.8 Draw the geometry
			const vk::DeviceSize offsets[1] = { 0 };
			vkInfo.cmd.bindVertexBuffers(0, 1, &vkInfo.vertex.buffer, offsets);
			vkInfo.cmd.draw(3, 1, 0, 0);
			vkInfo.cmd.endRenderPass();

			//15.9 Submit the command list
			vkInfo.cmd.end();

			vk::PipelineStageFlags dstStageMask[] = {
				vk::PipelineStageFlagBits::eBottomOfPipe
			};

			vkInfo.device.resetFences(1, &vkInfo.fence);

			auto submitInfo = vk::SubmitInfo()
				.setCommandBufferCount(1)
				.setPCommandBuffers(&vkInfo.cmd)
				.setWaitSemaphoreCount(1)
				.setPWaitSemaphores(&vkInfo.imageAcquiredSemaphore)
				.setPWaitDstStageMask(dstStageMask);
			vkInfo.queue.submit(1, &submitInfo, vkInfo.fence);
			
			//15.10 Wait for GPU to be finished
			vk::Result waitingRes;
			do
				waitingRes = vkInfo.device.waitForFences(1, &vkInfo.fence, VK_TRUE, UINT64_MAX);
			while (waitingRes == vk::Result::eTimeout);

			//15.11 Present
			auto presentInfo = vk::PresentInfoKHR()
				.setPImageIndices(&currentBuffer)
				.setSwapchainCount(1)
				.setPSwapchains(&vkInfo.swapchain);

			if (vkInfo.queue.presentKHR(&presentInfo) != vk::Result::eSuccess) {
				MessageBox(0, L"Present the render target failed!!!", 0, 0);
			}
		}

		break;
		case 1:
		{//处理消息
			while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (WM_QUIT != msg.message)
				{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
				else
				{
					bExit = TRUE;
				}
			}
		}
		break;
		default:
			break;
		}
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
