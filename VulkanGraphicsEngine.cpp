#include "framework.h"
#include "VulkanGraphicsEngine.h"

#include "Common/Texture.h"
#include "Common/FrameResource.h"
#include "RenderStructure.h"
#include "Common/Camera.h"
#include "Common/ShadowMap.h"
#include "Common/ParticleSystem.h"
#include "Common/PostProcessing.h"

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

	if (!vkInfo.input.Init(hInstance, hWnd)) {
		MessageBox(0, L"Init player input module failed!!!", 0, 0);
	}

	Camera mainCamera((float)windowWidth / (float)windowHeight);
	mainCamera.LookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	//创建一个点光
	Light pointLight;
	pointLight.position = glm::vec3(1.0f, 1.0f, 0.0f);
	pointLight.fallOffStart = 1.0f;
	pointLight.fallOffEnd = 10.0f;
	pointLight.strength = glm::vec3(1.0f, 1.0f, 1.0f);

	/*Set validation layer*/
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
	
	std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation",
		"VK_LAYER_LUNARG_monitor"
	};

	if (enableValidationLayers && !CheckValidationLayerSupport(validationLayers)) {
		MessageBox(0, L"Dont support validation layers!!!", 0, 0);
	}

	if (enableValidationLayers) {
		vkInfo.validationLayers = validationLayers;
		vkInfo.instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	
#ifndef NDEBUG
	vk::DynamicLoader dl;
	PFN_vkGetInstanceProcAddr GetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	PFN_vkDebugUtilsMessengerCallbackEXT CallbackFunc = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(DebugCallback);

	auto debugMessengerInfo = vk::DebugUtilsMessengerCreateInfoEXT()
		.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
		.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
		.setPfnUserCallback(CallbackFunc)
		.setPUserData(nullptr);
#endif

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
	
#ifndef NDEBUG
	//2.3 Create debug messengr
	vk::DispatchLoaderDynamic dispatch(vkInfo.instance, GetInstanceProcAddr);
	if (vkInfo.instance.createDebugUtilsMessengerEXT(&debugMessengerInfo, 0, &vkInfo.debugMessenger, dispatch) != vk::Result::eSuccess) {
		MessageBox(0, L"Create debug messenger failed!!!", 0, 0);
	}
#endif

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
	auto feature = vk::PhysicalDeviceFeatures()
		.setGeometryShader(VK_TRUE);

	float priorities[1] = { 0.0f };
	deviceQueueInfo.setQueueCount(1);
	deviceQueueInfo.setPQueuePriorities(priorities);
	deviceQueueInfo.setQueueFamilyIndex(vkInfo.graphicsQueueFamilyIndex);
	deviceInfo.setQueueCreateInfoCount(1);
	deviceInfo.setPQueueCreateInfos(&deviceQueueInfo);
	deviceInfo.setEnabledExtensionCount(vkInfo.deviceExtensions.size());
	deviceInfo.setPpEnabledExtensionNames(vkInfo.deviceExtensions.data());
	deviceInfo.setPEnabledFeatures(&feature);

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

	vkInfo.width = surfaceCapabilities.currentExtent.width;
	vkInfo.height = surfaceCapabilities.currentExtent.height;

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

	/*7. Create depth image*/
	vkInfo.depth.format = vk::Format::eD16Unorm;

	//7.1 Create image
	auto depthImageInfo = vk::ImageCreateInfo()
		.setArrayLayers(1)
		.setExtent(vk::Extent3D(vkInfo.width, vkInfo.height, 1))
		.setFormat(vkInfo.depth.format)
		.setImageType(vk::ImageType::e2D)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setMipLevels(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
	vkInfo.device.createImage(&depthImageInfo, 0, &vkInfo.depth.image);

	vk::MemoryRequirements depthImageReqs;
	vkInfo.device.getImageMemoryRequirements(vkInfo.depth.image, &depthImageReqs);

	auto depthMemAlloc = vk::MemoryAllocateInfo()
		.setAllocationSize(depthImageReqs.size);
	MemoryTypeFromProperties(vkInfo.gpu.getMemoryProperties(), depthImageReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, depthMemAlloc.memoryTypeIndex);
	vkInfo.device.allocateMemory(&depthMemAlloc, 0, &vkInfo.depth.memory);

	vkInfo.device.bindImageMemory(vkInfo.depth.image, vkInfo.depth.memory, 0);

	//7.2 Create image view
	auto depthImageViewInfo = vk::ImageViewCreateInfo()
		.setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA))
		.setFormat(vkInfo.depth.format)
		.setImage(vkInfo.depth.image)
		.setViewType(vk::ImageViewType::e2D)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
	vkInfo.device.createImageView(&depthImageViewInfo, 0, &vkInfo.depth.imageView);
	
	//创建用来渲染最初场景数据的图片
	auto sceneImageInfo = vk::ImageCreateInfo()
		.setArrayLayers(1)
		.setExtent(vk::Extent3D(vkInfo.width, vkInfo.height, 1))
		.setFormat(vkInfo.format)
		.setImageType(vk::ImageType::e2D)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setMipLevels(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
	vkInfo.device.createImage(&sceneImageInfo, 0, &vkInfo.scene.image);

	vk::MemoryRequirements sceneImageReqs;
	vkInfo.device.getImageMemoryRequirements(vkInfo.scene.image, &sceneImageReqs);

	auto sceneMemAlloc = vk::MemoryAllocateInfo()
		.setAllocationSize(sceneImageReqs.size);
	MemoryTypeFromProperties(vkInfo.gpu.getMemoryProperties(), sceneImageReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, sceneMemAlloc.memoryTypeIndex);
	vkInfo.device.allocateMemory(&sceneMemAlloc, 0, &vkInfo.scene.memory);

	vkInfo.device.bindImageMemory(vkInfo.scene.image, vkInfo.scene.memory, 0);

	auto sceneImageViewInfo = vk::ImageViewCreateInfo()
		.setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA))
		.setFormat(vkInfo.format)
		.setImage(vkInfo.scene.image)
		.setViewType(vk::ImageViewType::e2D)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
	vkInfo.device.createImageView(&sceneImageViewInfo, 0, &vkInfo.scene.imageView);

	//创建最初场景的渲染过程
	{
		auto colorAttachment = vk::AttachmentDescription()
			.setFormat(vkInfo.format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		auto depthAttachment = vk::AttachmentDescription()
			.setFormat(vkInfo.depth.format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		vk::AttachmentDescription attachments[] = {
			colorAttachment, depthAttachment
		};

		auto colorReference = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		auto depthReference = vk::AttachmentReference()
			.setAttachment(1)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		auto subpass = vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&colorReference)
			.setPDepthStencilAttachment(&depthReference);

		auto renderPassInfo = vk::RenderPassCreateInfo()
			.setAttachmentCount(2)
			.setPAttachments(attachments)
			.setSubpassCount(1)
			.setPSubpasses(&subpass);

		if (vkInfo.device.createRenderPass(&renderPassInfo, 0, &vkInfo.scenePass) != vk::Result::eSuccess) {
			MessageBox(0, L"Create render pass failed!!!", 0, 0);
		}
	}

	//绑定深度图和Scene视图为帧缓存
	{
		vk::ImageView framebufferView[2];
		framebufferView[0] = vkInfo.scene.imageView;
		framebufferView[1] = vkInfo.depth.imageView;
		auto framebufferInfo = vk::FramebufferCreateInfo()
			.setRenderPass(vkInfo.scenePass)
			.setAttachmentCount(2)
			.setPAttachments(framebufferView)
			.setWidth(vkInfo.width)
			.setHeight(vkInfo.height)
			.setLayers(1);
		vkInfo.device.createFramebuffer(&framebufferInfo, 0, &vkInfo.scene.framebuffer);
	}

	//创建用于融混后处理色彩的渲染过程
	{
		auto colorAttachment = vk::AttachmentDescription()
			.setFormat(vkInfo.format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

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

		vkInfo.device.createRenderPass(&renderPassInfo, 0, &vkInfo.combinePass);
	}

	//绑定交换链图像到帧缓存
	{
		vk::ImageView framebufferView;
		auto framebufferInfo = vk::FramebufferCreateInfo()
			.setRenderPass(vkInfo.combinePass)
			.setAttachmentCount(1)
			.setPAttachments(&framebufferView)
			.setWidth(vkInfo.width)
			.setHeight(vkInfo.height)
			.setLayers(1);

		vkInfo.combineFramebuffers.resize(vkInfo.frameCount);

		for (size_t i = 0; i < vkInfo.frameCount; i++) {
			framebufferView = vkInfo.swapchainImageViews[i];
			vkInfo.device.createFramebuffer(&framebufferInfo, 0, &vkInfo.combineFramebuffers[i]);
		}
	}

	//创建后处理系统
	PostProcessing::Bloom bloom;

/*=============================================================渲染相关事项=============================================================*/

	//加载模型
	SkinnedModel model("Assets\\skinnedModel.fbx");

	//获取所有的渲染项，构成一个Scene
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> geo;//存储顶点及索引缓冲
	std::vector<std::unique_ptr<Material>> materials;				   //存储所有材质
	std::vector<std::unique_ptr<RenderItem>> ritems;				   //存储所有渲染项
	BuildRenderItems(&vkInfo.device, vkInfo.gpu.getMemoryProperties(), model, geo, materials, ritems);

	uint32_t frameCount = 1;										   //帧缓存的数量，这里只使用1个
	uint32_t skinnedObjectCount = 1 * frameCount;					   //骨架的数量
	uint32_t passCount = 2 * frameCount;							   //Pass常量的数量
	uint32_t objectCount = ritems.size() * frameCount;				   //物体的数量，通常为渲染项数量*帧缓冲数量
	uint32_t descCount = passCount + objectCount;					   //描述符的数量，为物体数+帧缓存数

	//按不同的Layer划分渲染项（使用不同的管线）
	std::vector<RenderItem*> renderLayers[(int)RenderLayer::layerCount];
	for (uint32_t i = 0; i < ritems.size(); i++)
		renderLayers[(int)ritems[i]->layer].push_back(ritems[i].get());

	//初始化帧缓冲
	auto frameResource = std::make_unique<FrameResource>(&vkInfo.device, vkInfo.gpu.getMemoryProperties(), 2, ritems.size(), materials.size(), skinnedObjectCount);

	//初始化骨骼动画
	auto skinnedModelInst = std::make_unique<SkinnedModelInstance>();
	SkinnedData skinndData;
	{
		std::vector<int> boneHierarchy;
		std::vector<glm::mat4x4> boneOffsets;
		std::vector<glm::mat4x4> defaultTransform;
		model.GetBoneHierarchy(boneHierarchy);
		model.GetNodeOffsets(defaultTransform);
		model.GetBoneOffsets(boneOffsets);

		std::unordered_map<std::string, AnimationClip> animations;
		model.GetAnimations(animations);

		for (size_t i = 0; i < defaultTransform.size(); i++)
			animations["default"].boneAnimations[i].defaultTransform = defaultTransform[i];

		skinndData.Set(boneHierarchy, boneOffsets, animations);

		skinnedModelInst->skinnedInfo = &skinndData;
		skinnedModelInst->timePos = 0.0f;
		skinnedModelInst->finalTransforms.resize(skinndData.GetBoneCount());
		skinnedModelInst->clipName = "default";
	}

	//初始化火焰的粒子系统
	ParticleSystem flameParticle;
	ParticleSystem::Property property;
	property.maxSize = 5.0f;
	property.minSize = 4.0f;
	property.minLastTime = 0.5f;
	property.maxLastTime = 2.0f;
	property.maxVelocity = glm::vec3(0.0f, 1.0f, 0.0f);
	property.minVelocity = glm::vec3(0.0f, 1.0f, 0.0f);
	property.minAlpha = 0.9f;
	property.maxAlpha = 1.0f;
	property.colorFadeSpeed = 0.5f;
	property.sizeFadeSpeed = 1.0f;
	property.color = glm::vec3(0.0f, 0.0f, 0.0f);
	flameParticle.SetParticleProperty(property);
	ParticleSystem::Emitter emitter;
	emitter.maxParticleNum = 10;
	emitter.position = glm::vec3(0.0f, 0.0f, 0.0f);
	emitter.radius = 0.3f;
	flameParticle.SetEmitterProperty(emitter);
	ParticleSystem::Texture particleTexture;
	particleTexture.splitX = 7;
	particleTexture.splitY = 7;
	particleTexture.texCount = 7 * 7;
	flameParticle.SetTextureProperty(particleTexture);
	ParticleSystem::SubParticle subParticle;
	subParticle.color = glm::vec3(0.0f, 0.0f, 0.0f);
	subParticle.lastTime = 2.0f;
	subParticle.texture.splitX = 6;
	subParticle.texture.splitY = 6;
	subParticle.texture.texCount = 6 * 6;
	subParticle.size = 2.0f;
	subParticle.used = true;
	flameParticle.SetSubParticle(subParticle);
	flameParticle.PrepareParticles(&vkInfo.device, vkInfo.gpu.getMemoryProperties());

	//将所有要用到的贴图并放在一个vector容器里
	std::vector<Texture> textures;

	//贴图的目录
	std::vector<std::string> texturePath = {
		"Assets\\icon.jpg",
		"Assets\\brickTexture.jpg",
		"Assets\\flame.png",
		"Assets\\SmokeLoop.png"
	};

	//加载模型的所有贴图
	for (uint32_t i = 0; i < model.texturePath.size(); i++) {
		Texture texture;
		LoadPixelWithSTB(model.texturePath[i].c_str(), 32, texture, &vkInfo.device, vkInfo.gpu.getMemoryProperties());
		texture.SetupImage(&vkInfo.device, vkInfo.gpu.getMemoryProperties(), vkInfo.cmdPool, &vkInfo.queue);
		textures.push_back(texture);
	}

	//加载目录下的所有贴图
	for (uint32_t i = 0; i < texturePath.size(); i++) {
		Texture texture;
		LoadPixelWithSTB(texturePath[i].c_str(), 32, texture, &vkInfo.device, vkInfo.gpu.getMemoryProperties());
		texture.SetupImage(&vkInfo.device, vkInfo.gpu.getMemoryProperties(), vkInfo.cmdPool, &vkInfo.queue);
		textures.push_back(texture);
	}

	//加载立方体贴图
	{
		Texture texture;
		LoadCubeMapWithWIC(L"Assets\\skybox.png", GUID_WICPixelFormat32bppRGBA, texture, &vkInfo.device, vkInfo.gpu.getMemoryProperties());
		texture.SetupImage(&vkInfo.device, vkInfo.gpu.getMemoryProperties(), vkInfo.cmdPool, &vkInfo.queue);
		textures.push_back(texture);
	}

	//清理缓存
	for (uint32_t i = 0; i < textures.size(); i++) {
		textures[i].CleanUploader(&vkInfo.device);
	}

	/*初始化阴影贴图*/
	ShadowMap shadowMap(&vkInfo.device, vkInfo.gpu.getMemoryProperties(), vkInfo.width, vkInfo.height);
	shadowMap.SetLightTransformMatrix(glm::normalize(glm::vec3(-1.0f, 0.0f, 1.0f) - pointLight.position), 100.0f);

	//创建通用的采样器
	vk::Sampler repeatSampler;
	vk::Sampler borderSampler;
	{
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
			.setUnnormalizedCoordinates(VK_FALSE);

		samplerInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);
		samplerInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);
		samplerInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);
		vkInfo.device.createSampler(&samplerInfo, 0, &repeatSampler);

		samplerInfo.setAddressModeU(vk::SamplerAddressMode::eClampToBorder);
		samplerInfo.setAddressModeV(vk::SamplerAddressMode::eClampToBorder);
		samplerInfo.setAddressModeW(vk::SamplerAddressMode::eClampToBorder);
		vkInfo.device.createSampler(&samplerInfo, 0, &borderSampler);
	}

	//创建用于阴影贴图的比较采样器
	vk::Sampler comparisonSampler;
	{
		auto samplerInfo = vk::SamplerCreateInfo()
			.setAddressModeU(vk::SamplerAddressMode::eClampToBorder)
			.setAddressModeV(vk::SamplerAddressMode::eClampToBorder)
			.setAddressModeW(vk::SamplerAddressMode::eClampToBorder)
			.setAnisotropyEnable(VK_FALSE)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setCompareEnable(VK_TRUE)
			.setCompareOp(vk::CompareOp::eLessOrEqual)
			.setMagFilter(vk::Filter::eLinear)
			.setMaxLod(1.0f)
			.setMinLod(0.0f)
			.setMipLodBias(0.0f)
			.setMinFilter(vk::Filter::eLinear)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setUnnormalizedCoordinates(VK_FALSE);
		vkInfo.device.createSampler(&samplerInfo, 0, &comparisonSampler);
	}

	//第一个管线布局：世界矩阵,纹理和材质常量
	auto objCBBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	auto textureBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampledImage)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto materialCBBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(2)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	vk::DescriptorSetLayoutBinding layoutBindingObject[] = {
		objCBBinding, textureBinding, materialCBBinding
	};

	//第二个管线布局：Pass常量和采样器
	auto passCBBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry | vk::ShaderStageFlagBits::eFragment);

	auto repeatSamplerBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto borderSamplerBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(2)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	vk::DescriptorSetLayoutBinding layoutBindingPass[] = {
		passCBBinding, repeatSamplerBinding, borderSamplerBinding
	};

	//第三个管线布局：比较采样器和阴影贴图
	auto shadowSamplerBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto shadowMapBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampledImage)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	vk::DescriptorSetLayoutBinding layoutBindingShadow[] = {
		shadowSamplerBinding, shadowMapBinding
	};

	//第四个管线布局：骨骼的变换矩阵
	auto layoutBindingSkinned = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	//融混后处理效果的管线布局：采样器和源贴图
	auto combineSamplerBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto combineImageBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampledImage)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	vk::DescriptorSetLayoutBinding layoutBindingCombine[] = {
		combineSamplerBinding, combineImageBinding
	};

	//为渲染管线提供管线布局
	vkInfo.descSetLayout.resize(4);

	auto descLayoutInfo_obj = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(3)
		.setPBindings(layoutBindingObject);
	vkInfo.device.createDescriptorSetLayout(&descLayoutInfo_obj, 0, &vkInfo.descSetLayout[0]);

	auto descLayoutInfo_pass = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(3)
		.setPBindings(layoutBindingPass);
	vkInfo.device.createDescriptorSetLayout(&descLayoutInfo_pass, 0, &vkInfo.descSetLayout[1]);

	auto descLayoutInfo_shadow = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(2)
		.setPBindings(layoutBindingShadow);
	vkInfo.device.createDescriptorSetLayout(&descLayoutInfo_shadow, 0, &vkInfo.descSetLayout[2]);

	auto descLayoutInfo_skinned = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(1)
		.setPBindings(&layoutBindingSkinned);
	vkInfo.device.createDescriptorSetLayout(&descLayoutInfo_skinned, 0, &vkInfo.descSetLayout[3]);

	//为融混管线提供管线布局
	auto descLayoutInfo_combine = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(2)
		.setPBindings(layoutBindingCombine);
	vkInfo.device.createDescriptorSetLayout(&descLayoutInfo_combine, 0, &vkInfo.combinePassLayout);

	//为描述符的分配提供布局
	std::vector<vk::DescriptorSetLayout> descLayout(descCount + frameCount + skinnedObjectCount);
	for (uint32_t i = 0; i < objectCount; i++)
		descLayout[i] = vkInfo.descSetLayout[0];
	for (uint32_t i = 0; i < passCount; i++) {
		uint32_t index = i + objectCount;
		descLayout[index] = vkInfo.descSetLayout[1];
	}
	for (uint32_t i = 0; i < frameCount; i++) {
		uint32_t index = i + objectCount + passCount;
		descLayout[index] = vkInfo.descSetLayout[2];
	}
	for (uint32_t i = 0; i < skinnedObjectCount; i++) {
		uint32_t index = i + objectCount + passCount + frameCount;
		descLayout[index] = vkInfo.descSetLayout[3];
	}

	//创建描述符池
	vk::DescriptorPoolSize typeCount[3];
	typeCount[0].setType(vk::DescriptorType::eUniformBuffer);
	typeCount[0].setDescriptorCount(descCount + objectCount + skinnedObjectCount);
	typeCount[1].setType(vk::DescriptorType::eSampledImage);
	typeCount[1].setDescriptorCount(objectCount + frameCount + bloom.GetRequiredDescCount() + 1);
	typeCount[2].setType(vk::DescriptorType::eSampler);
	typeCount[2].setDescriptorCount(passCount * 2 + frameCount + bloom.GetRequiredDescCount() + 1);

	auto descriptorPoolInfo = vk::DescriptorPoolCreateInfo()
		.setMaxSets(descCount + frameCount + skinnedObjectCount + bloom.GetRequiredDescCount() + 1)
		.setPoolSizeCount(3)
		.setPPoolSizes(typeCount);
	vkInfo.device.createDescriptorPool(&descriptorPoolInfo, 0, &vkInfo.descPool);

	//分配descCount个描述符
	vkInfo.descSets.resize(descCount + frameCount + skinnedObjectCount);

	auto descSetAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(vkInfo.descPool)
		.setDescriptorSetCount(descCount + frameCount + skinnedObjectCount)
		.setPSetLayouts(descLayout.data());
	vkInfo.device.allocateDescriptorSets(&descSetAllocInfo, vkInfo.descSets.data());
	
	//为融混分配一个描述符
	vkInfo.combinePassDescSets.resize(1);

	descSetAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(vkInfo.descPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&vkInfo.combinePassLayout);
	vkInfo.device.allocateDescriptorSets(&descSetAllocInfo, vkInfo.combinePassDescSets.data());

	//初始化后处理系统
	bloom.Init(vkInfo, vkInfo.scene.imageView);

	//更新每一个描述符
	for (uint32_t i = 0; i < objectCount; i++) {
		auto descriptrorObjCBInfo = vk::DescriptorBufferInfo()
			.setBuffer(frameResource->objCB[ritems[i]->objCBIndex]->GetBuffer())
			.setOffset(0)
			.setRange(sizeof(ObjectConstants));

		auto descriptorImageInfo = vk::DescriptorImageInfo()
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(textures[ritems[i]->material->diffuseIndex].GetImageView(&vkInfo.device));

		auto descriptorMatCBInfo = vk::DescriptorBufferInfo()
			.setBuffer(frameResource->matCB[ritems[i]->material->matCBIndex]->GetBuffer())
			.setOffset(0)
			.setRange(sizeof(MaterialConstants));

		vk::WriteDescriptorSet descSetWrites[3];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(0);
		descSetWrites[0].setDstSet(vkInfo.descSets[i]);
		descSetWrites[0].setPBufferInfo(&descriptrorObjCBInfo);
		descSetWrites[1].setDescriptorCount(1);
		descSetWrites[1].setDescriptorType(vk::DescriptorType::eSampledImage);
		descSetWrites[1].setDstArrayElement(0);
		descSetWrites[1].setDstBinding(1);
		descSetWrites[1].setDstSet(vkInfo.descSets[i]);
		descSetWrites[1].setPImageInfo(&descriptorImageInfo);
		descSetWrites[2].setDescriptorCount(1);
		descSetWrites[2].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		descSetWrites[2].setDstArrayElement(0);
		descSetWrites[2].setDstBinding(2);
		descSetWrites[2].setDstSet(vkInfo.descSets[i]);
		descSetWrites[2].setPBufferInfo(&descriptorMatCBInfo);
		vkInfo.device.updateDescriptorSets(3, descSetWrites, 0, 0);
	}

	for (uint32_t i = 0; i < passCount; i++) {
		uint32_t index = i + objectCount;

		auto descriptrorPassCBInfo = vk::DescriptorBufferInfo()
			.setBuffer(frameResource->passCB[i]->GetBuffer())
			.setOffset(0)
			.setRange(sizeof(PassConstants));

		auto descriptorRepeatSamplerInfo = vk::DescriptorImageInfo()
			.setSampler(repeatSampler);

		auto descriptorBorderSamplerInfo = vk::DescriptorImageInfo()
			.setSampler(borderSampler);

		vk::WriteDescriptorSet descSetWrites[3];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(0);
		descSetWrites[0].setDstSet(vkInfo.descSets[index]);
		descSetWrites[0].setPBufferInfo(&descriptrorPassCBInfo);
		descSetWrites[1].setDescriptorCount(1);
		descSetWrites[1].setDescriptorType(vk::DescriptorType::eSampler);
		descSetWrites[1].setDstArrayElement(0);
		descSetWrites[1].setDstBinding(1);
		descSetWrites[1].setDstSet(vkInfo.descSets[index]);
		descSetWrites[1].setPImageInfo(&descriptorRepeatSamplerInfo);
		descSetWrites[2].setDescriptorCount(1);
		descSetWrites[2].setDescriptorType(vk::DescriptorType::eSampler);
		descSetWrites[2].setDstArrayElement(0);
		descSetWrites[2].setDstBinding(2);
		descSetWrites[2].setDstSet(vkInfo.descSets[index]);
		descSetWrites[2].setPImageInfo(&descriptorBorderSamplerInfo);
		vkInfo.device.updateDescriptorSets(3, descSetWrites, 0, 0);
	}

	{
		auto descriptorSamplerInfo = vk::DescriptorImageInfo()
			.setSampler(comparisonSampler);

		auto descriptorShadowMapInfo = vk::DescriptorImageInfo()
			.setImageView(shadowMap.GetImageView())
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::WriteDescriptorSet descSetWrites[2];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eSampler);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(0);
		descSetWrites[0].setDstSet(vkInfo.descSets[descCount]);
		descSetWrites[0].setPImageInfo(&descriptorSamplerInfo);
		descSetWrites[1].setDescriptorCount(1);
		descSetWrites[1].setDescriptorType(vk::DescriptorType::eSampledImage);
		descSetWrites[1].setDstArrayElement(0);
		descSetWrites[1].setDstBinding(1);
		descSetWrites[1].setDstSet(vkInfo.descSets[descCount]);
		descSetWrites[1].setPImageInfo(&descriptorShadowMapInfo);
		vkInfo.device.updateDescriptorSets(2, descSetWrites, 0, 0);
	}

	{
		uint32_t index = descCount + frameCount;

		auto descriptrorSkinnedCBInfo = vk::DescriptorBufferInfo()
			.setBuffer(frameResource->skinnedCB[0]->GetBuffer())
			.setOffset(0)
			.setRange(sizeof(SkinnedConstants));

		vk::WriteDescriptorSet descSetWrites[1];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(0);
		descSetWrites[0].setDstSet(vkInfo.descSets[index]);
		descSetWrites[0].setPBufferInfo(&descriptrorSkinnedCBInfo);
		vkInfo.device.updateDescriptorSets(1, descSetWrites, 0, 0);
	}

	{
		auto descriptorSamplerInfo = vk::DescriptorImageInfo()
			.setSampler(repeatSampler);

		auto descriptorImageInfo = vk::DescriptorImageInfo()
			.setImageView(bloom.GetImageView())
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::WriteDescriptorSet descSetWrites[2];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eSampler);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(0);
		descSetWrites[0].setDstSet(vkInfo.combinePassDescSets[0]);
		descSetWrites[0].setPImageInfo(&descriptorSamplerInfo);
		descSetWrites[1].setDescriptorCount(1);
		descSetWrites[1].setDescriptorType(vk::DescriptorType::eSampledImage);
		descSetWrites[1].setDstArrayElement(0);
		descSetWrites[1].setDstBinding(1);
		descSetWrites[1].setDstSet(vkInfo.combinePassDescSets[0]);
		descSetWrites[1].setPImageInfo(&descriptorImageInfo);
		vkInfo.device.updateDescriptorSets(2, descSetWrites, 0, 0);
	}

	//顶点输入装配属性
	vkInfo.vertex.binding.setBinding(0);
	vkInfo.vertex.binding.setInputRate(vk::VertexInputRate::eVertex);
	vkInfo.vertex.binding.setStride(sizeof(Vertex));

	vkInfo.vertex.attrib.resize(3);

	vkInfo.vertex.attrib[0].setBinding(0);
	vkInfo.vertex.attrib[0].setFormat(vk::Format::eR32G32B32Sfloat);
	vkInfo.vertex.attrib[0].setLocation(0);
	vkInfo.vertex.attrib[0].setOffset(0);

	vkInfo.vertex.attrib[1].setBinding(0);
	vkInfo.vertex.attrib[1].setFormat(vk::Format::eR32G32Sfloat);
	vkInfo.vertex.attrib[1].setLocation(1);
	vkInfo.vertex.attrib[1].setOffset(sizeof(glm::vec3));

	vkInfo.vertex.attrib[2].setBinding(0);
	vkInfo.vertex.attrib[2].setFormat(vk::Format::eR32G32B32Sfloat);
	vkInfo.vertex.attrib[2].setLocation(2);
	vkInfo.vertex.attrib[2].setOffset(sizeof(glm::vec3) + sizeof(glm::vec2));

	//蒙皮网格的顶点输入装配属性
	vk::VertexInputBindingDescription skinnedBinding;
	std::vector<vk::VertexInputAttributeDescription> skinnedAttrib;

	skinnedBinding.setBinding(0);
	skinnedBinding.setInputRate(vk::VertexInputRate::eVertex);
	skinnedBinding.setStride(sizeof(SkinnedVertex));

	skinnedAttrib.resize(5);

	skinnedAttrib[0].setBinding(0);
	skinnedAttrib[0].setFormat(vk::Format::eR32G32B32Sfloat);
	skinnedAttrib[0].setLocation(0);
	skinnedAttrib[0].setOffset(0);

	skinnedAttrib[1].setBinding(0);
	skinnedAttrib[1].setFormat(vk::Format::eR32G32Sfloat);
	skinnedAttrib[1].setLocation(1);
	skinnedAttrib[1].setOffset(sizeof(glm::vec3));

	skinnedAttrib[2].setBinding(0);
	skinnedAttrib[2].setFormat(vk::Format::eR32G32B32Sfloat);
	skinnedAttrib[2].setLocation(2);
	skinnedAttrib[2].setOffset(sizeof(glm::vec3) + sizeof(glm::vec2));

	skinnedAttrib[3].setBinding(0);
	skinnedAttrib[3].setFormat(vk::Format::eR32G32B32Sfloat);
	skinnedAttrib[3].setLocation(3);
	skinnedAttrib[3].setOffset(2 * sizeof(glm::vec3) + sizeof(glm::vec2));

	skinnedAttrib[4].setBinding(0);
	skinnedAttrib[4].setFormat(vk::Format::eR32G32B32Uint);
	skinnedAttrib[4].setLocation(4);
	skinnedAttrib[4].setOffset(3 * sizeof(glm::vec3) + sizeof(glm::vec2));

	//粒子的顶点输入装配属性
	vk::VertexInputBindingDescription particleBinding;
	std::vector<vk::VertexInputAttributeDescription> particleAttrib;

	particleBinding.setBinding(0);
	particleBinding.setInputRate(vk::VertexInputRate::eVertex);
	particleBinding.setStride(sizeof(ParticleSystem::Particle));

	particleAttrib.resize(5);

	//glm::vec3 position
	particleAttrib[0].setBinding(0);
	particleAttrib[0].setFormat(vk::Format::eR32G32B32Sfloat);
	particleAttrib[0].setLocation(0);
	particleAttrib[0].setOffset(0);

	//float size
	particleAttrib[1].setBinding(0);
	particleAttrib[1].setFormat(vk::Format::eR32Sfloat);
	particleAttrib[1].setLocation(1);
	particleAttrib[1].setOffset(sizeof(glm::vec3));

	//glm::vec4 color
	particleAttrib[2].setBinding(0);
	particleAttrib[2].setFormat(vk::Format::eR32G32B32A32Sfloat);
	particleAttrib[2].setLocation(2);
	particleAttrib[2].setOffset(sizeof(glm::vec3) + sizeof(float));

	//glm::vec4 texCoord
	particleAttrib[3].setBinding(0);
	particleAttrib[3].setFormat(vk::Format::eR32G32B32A32Sfloat);
	particleAttrib[3].setLocation(3);
	particleAttrib[3].setOffset(sizeof(glm::vec4) + sizeof(glm::vec3) + sizeof(float));

/*=========================================================================================================================================*/

	/*13. Compile shader module*/

	//13.1 Create shader module
	auto vsModule = CreateShaderModule("Shaders\\vertex.spv", vkInfo.device);
	auto psModule = CreateShaderModule("Shaders\\fragment.spv", vkInfo.device);

	//13.2 Create pipeline shader module
	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderInfo(2);

	pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(vsModule)
		.setStage(vk::ShaderStageFlagBits::eVertex);

	pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(psModule)
		.setStage(vk::ShaderStageFlagBits::eFragment);

	/*16. Create a graphics pipeline state*/

	//16.1 Dynamic state
	auto dynamicInfo = vk::PipelineDynamicStateCreateInfo();
	std::vector<vk::DynamicState> dynamicStates;

	//16.2 Vertex input state
	auto viInfo = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptionCount(1)
		.setPVertexBindingDescriptions(&vkInfo.vertex.binding)
		.setVertexAttributeDescriptionCount(3)
		.setPVertexAttributeDescriptions(vkInfo.vertex.attrib.data());

	//16.3 Input assembly state
	auto iaInfo = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(vk::PrimitiveTopology::eTriangleList)
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

	//16.7 Depth stencil state
	auto dsInfo = vk::PipelineDepthStencilStateCreateInfo()
		.setDepthTestEnable(VK_TRUE)
		.setDepthWriteEnable(VK_TRUE)
		.setDepthCompareOp(vk::CompareOp::eLess)
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
		.setSetLayoutCount(vkInfo.descSetLayout.size())
		.setPSetLayouts(vkInfo.descSetLayout.data());
	if (vkInfo.device.createPipelineLayout(&plInfo, 0, &vkInfo.pipelineLayout["scene"]) != vk::Result::eSuccess) {
		MessageBox(0, L"Create pipeline layout failed!!!", 0, 0);
	}

	//16.11 Create pipeline state
	vkInfo.pipelines["opaque"] = CreateGraphicsPipeline(vkInfo.device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo.pipelineLayout["scene"], pipelineShaderInfo, vkInfo.scenePass);
	vkInfo.device.destroyShaderModule(vsModule);
	vkInfo.device.destroyShaderModule(psModule);
	
	//编译用于蒙皮动画的着色器
	vsModule = CreateShaderModule("Shaders\\skinnedVS.spv", vkInfo.device);
	psModule = CreateShaderModule("Shaders\\skinnedPS.spv", vkInfo.device);

	pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(vsModule)
		.setStage(vk::ShaderStageFlagBits::eVertex);

	pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(psModule)
		.setStage(vk::ShaderStageFlagBits::eFragment);

	//创建用于蒙皮动画的管线
	auto skinnedviInfo = viInfo;
	skinnedviInfo.setVertexBindingDescriptionCount(1);
	skinnedviInfo.setPVertexBindingDescriptions(&skinnedBinding);
	skinnedviInfo.setVertexAttributeDescriptionCount(skinnedAttrib.size());
	skinnedviInfo.setPVertexAttributeDescriptions(skinnedAttrib.data());

	vkInfo.pipelines["skinned"] = CreateGraphicsPipeline(vkInfo.device, dynamicInfo, skinnedviInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo.pipelineLayout["scene"], pipelineShaderInfo, vkInfo.scenePass);
	vkInfo.device.destroyShaderModule(vsModule);
	vkInfo.device.destroyShaderModule(psModule);

	//编译用于天空球的着色器
	vsModule = CreateShaderModule("Shaders\\skyboxVS.spv", vkInfo.device);
	psModule = CreateShaderModule("Shaders\\skyboxPS.spv", vkInfo.device);

	pipelineShaderInfo[0].setModule(vsModule);
	pipelineShaderInfo[1].setModule(psModule);

	//创建用于绘制天空球的管线
	dsInfo.setDepthCompareOp(vk::CompareOp::eLessOrEqual);

	vkInfo.pipelines["skybox"] = CreateGraphicsPipeline(vkInfo.device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo.pipelineLayout["scene"], pipelineShaderInfo, vkInfo.scenePass);

	vkInfo.device.destroyShaderModule(vsModule);
	vkInfo.device.destroyShaderModule(psModule);

	//编译用于阴影贴图的着色器
	vsModule = CreateShaderModule("Shaders\\shadowVS.spv", vkInfo.device);
	psModule = CreateShaderModule("Shaders\\shadowPS.spv", vkInfo.device);

	pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(vsModule)
		.setStage(vk::ShaderStageFlagBits::eVertex);
	pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(psModule)
		.setStage(vk::ShaderStageFlagBits::eFragment);

	//创建用于生成阴影图的管线
	rsInfo.setDepthBiasEnable(VK_TRUE);
	rsInfo.setDepthBiasSlopeFactor(1.0f);
	rsInfo.setDepthBiasClamp(0.0f);
	rsInfo.setDepthBiasConstantFactor(20);
	cbInfo.setAttachmentCount(0);
	cbInfo.setPAttachments(0);

	vkInfo.pipelines["shadow"] = CreateGraphicsPipeline(vkInfo.device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo.pipelineLayout["scene"], pipelineShaderInfo, shadowMap.GetRenderPass());
	vkInfo.device.destroyShaderModule(vsModule);

	//编译用于蒙皮动画的阴影着色器
	vsModule = CreateShaderModule("Shaders\\shadowSkinnedVS.spv", vkInfo.device);
	pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(vsModule)
		.setStage(vk::ShaderStageFlagBits::eVertex);

	//创建用于蒙皮动画的阴影管线
	vkInfo.pipelines["skinnedShadow"] = CreateGraphicsPipeline(vkInfo.device, dynamicInfo, skinnedviInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo.pipelineLayout["scene"], pipelineShaderInfo, shadowMap.GetRenderPass());
	vkInfo.device.destroyShaderModule(psModule);

	//编译用于图像混合的着色器
	vsModule = CreateShaderModule("Shaders\\bloomVS.spv", vkInfo.device);
	psModule = CreateShaderModule("Shaders\\combine.spv", vkInfo.device);

	pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(vsModule)
		.setStage(vk::ShaderStageFlagBits::eVertex);
	pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(psModule)
		.setStage(vk::ShaderStageFlagBits::eFragment);

	//创建管线布局
	auto combinePipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(0)
		.setSetLayoutCount(1)
		.setPSetLayouts(&vkInfo.combinePassLayout);
	vkInfo.device.createPipelineLayout(&combinePipelineLayoutInfo, 0, &vkInfo.pipelineLayout["combine"]);

	//创建用于图像混合的管线
	attState.setBlendEnable(VK_TRUE);
	attState.setColorBlendOp(vk::BlendOp::eAdd);
	attState.setSrcColorBlendFactor(vk::BlendFactor::eOne);
	attState.setDstColorBlendFactor(vk::BlendFactor::eOne);
	attState.setAlphaBlendOp(vk::BlendOp::eAdd);
	attState.setSrcAlphaBlendFactor(vk::BlendFactor::eZero);
	attState.setDstAlphaBlendFactor(vk::BlendFactor::eOne);

	cbInfo = vk::PipelineColorBlendStateCreateInfo()
		.setLogicOpEnable(VK_FALSE)
		.setAttachmentCount(1)
		.setPAttachments(&attState)
		.setLogicOp(vk::LogicOp::eNoOp);

	iaInfo.setTopology(vk::PrimitiveTopology::eTriangleStrip);

	viInfo = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptionCount(0)
		.setPVertexBindingDescriptions(0)
		.setVertexAttributeDescriptionCount(0)
		.setPVertexAttributeDescriptions(0);

	vkInfo.pipelines["combine"] = CreateGraphicsPipeline(vkInfo.device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo.pipelineLayout["combine"], pipelineShaderInfo, vkInfo.combinePass);

	vkInfo.device.destroyShaderModule(vsModule);
	vkInfo.device.destroyShaderModule(psModule);

	//编译用于粒子效果的着色器
	vsModule = CreateShaderModule("Shaders\\particleVS.spv", vkInfo.device);
	vk::ShaderModule gsModule = CreateShaderModule("Shaders\\particleGS.spv", vkInfo.device);
	psModule = CreateShaderModule("Shaders\\particlePS.spv", vkInfo.device);

	pipelineShaderInfo.resize(3);
	pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(vsModule)
		.setStage(vk::ShaderStageFlagBits::eVertex);
	pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(gsModule)
		.setStage(vk::ShaderStageFlagBits::eGeometry);
	pipelineShaderInfo[2] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(psModule)
		.setStage(vk::ShaderStageFlagBits::eFragment);

	//创建用于粒子效果的管线
	dsInfo = vk::PipelineDepthStencilStateCreateInfo()
		.setDepthTestEnable(VK_TRUE)
		.setDepthWriteEnable(VK_FALSE)
		.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setStencilTestEnable(VK_FALSE);

	attState.setBlendEnable(VK_TRUE);
	attState.setColorBlendOp(vk::BlendOp::eAdd);
	attState.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
	attState.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
	attState.setAlphaBlendOp(vk::BlendOp::eAdd);
	attState.setSrcAlphaBlendFactor(vk::BlendFactor::eZero);
	attState.setDstAlphaBlendFactor(vk::BlendFactor::eOne);

	cbInfo = vk::PipelineColorBlendStateCreateInfo()
		.setLogicOpEnable(VK_FALSE)
		.setAttachmentCount(1)
		.setPAttachments(&attState)
		.setLogicOp(vk::LogicOp::eNoOp);

	iaInfo.setTopology(vk::PrimitiveTopology::ePointList);

	viInfo = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptionCount(1)
		.setPVertexBindingDescriptions(&particleBinding)
		.setVertexAttributeDescriptionCount(4)
		.setPVertexAttributeDescriptions(particleAttrib.data());

	vkInfo.pipelines["smoke"] = CreateGraphicsPipeline(vkInfo.device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo.pipelineLayout["scene"], pipelineShaderInfo, vkInfo.scenePass);

	attState.setDstColorBlendFactor(vk::BlendFactor::eOne);

	vkInfo.pipelines["flame"] = CreateGraphicsPipeline(vkInfo.device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo.pipelineLayout["scene"], pipelineShaderInfo, vkInfo.scenePass);

	vkInfo.device.destroyShaderModule(vsModule);
	vkInfo.device.destroyShaderModule(gsModule);
	vkInfo.device.destroyShaderModule(psModule);

	/*17. Draw the geometry*/

	//17.1 Prepare semaphore and fence
	auto semaphoreInfo = vk::SemaphoreCreateInfo();
	if (vkInfo.device.createSemaphore(&semaphoreInfo, 0, &vkInfo.imageAcquiredSemaphore) != vk::Result::eSuccess) {
		MessageBox(0, L"Create image acquired semaphore failed!!!", 0, 0);
	}
	
	auto fenceInfo = vk::FenceCreateInfo();
	if (vkInfo.device.createFence(&fenceInfo, 0, &vkInfo.fence) != vk::Result::eSuccess) {
		MessageBox(0, L"Create fence failed!!!", 0, 0);
	}

	//17.2 Prepare render pass begin info
	auto renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setRenderArea(vk::Rect2D(vk::Offset2D(0.0f, 0.0f), vk::Extent2D(vkInfo.width, vkInfo.height)));

	//17.3 Begin record commands
	auto cmdBeginInfo = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	float rotate = 0.0f;
	float deltaTime = 0.01f;

	//更新MaterialConstants
	for (uint32_t i = 0; i < materials.size(); i++) {
		MaterialConstants materialConstants;
		materialConstants.diffuseAlbedo = materials[i]->diffuseAlbedo;
		materialConstants.fresnelR0 = materials[i]->fresnelR0;
		materialConstants.matTransform = materials[i]->matTransform;
		materialConstants.roughness = materials[i]->roughness;
		frameResource->matCB[i]->CopyData(&vkInfo.device, 0, 1, &materialConstants);
	}

	//更新SkinnedConstants
	skinnedModelInst->skinnedInfo->GetFinalTransform("default", 0.0f, skinnedModelInst->finalTransforms);
	SkinnedConstants skinnedConstants;
	std::copy(std::begin(skinnedModelInst->finalTransforms), std::end(skinnedModelInst->finalTransforms), skinnedConstants.boneTransforms);
	frameResource->skinnedCB[0]->CopyData(&vkInfo.device, 0, 1, &skinnedConstants);

	//Update
	HANDLE phWait = CreateWaitableTimer(NULL, FALSE, NULL);
	LARGE_INTEGER liDueTime = {};
	liDueTime.QuadPart = -1i64;
	SetWaitableTimer(phWait, &liDueTime, 1, NULL, NULL, 0);

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
			//Update
			vkInfo.input.Update();
			rotate += glm::pi<float>() * deltaTime;
			flameParticle.UpdateParticles(deltaTime, &vkInfo.device);
			
			/*更新摄像机*/
			float rotateSpeed = glm::pi<float>() * 0.007f;
			float moveSpeed = 0.05f;

			float upMove = (vkInfo.input.GetKey(DIK_UP) ? 1.0f : 0.0f) - (vkInfo.input.GetKey(DIK_DOWN) ? 1.0f : 0.0f);
			float rightMove = (vkInfo.input.GetKey(DIK_RIGHT) ? 1.0f : 0.0f) - (vkInfo.input.GetKey(DIK_LEFT) ? 1.0f : 0.0f);

			float upRotate = (vkInfo.input.GetKey(DIK_W) ? 1.0f : 0.0f) - (vkInfo.input.GetKey(DIK_S) ? 1.0f : 0.0f);
			float rightRotate = (vkInfo.input.GetKey(DIK_D) ? 1.0f : 0.0f) - (vkInfo.input.GetKey(DIK_A) ? 1.0f : 0.0f);

			mainCamera.Walk(upMove * moveSpeed);
			mainCamera.Strafe(rightMove * moveSpeed);
			mainCamera.Pitch(-upRotate * rotateSpeed);
			mainCamera.RotateY(rightRotate * rotateSpeed);

			mainCamera.UpdataViewMatrix();

			//改变物体的世界矩阵
			ritems[model.renderInfo.size()]->worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 2.0f)) * glm::rotate(glm::mat4(1.0f), rotate, glm::vec3(0.0f, 1.0f, 0.0f));

			//更新PassConstants
			PassConstants passConstants;
			passConstants.projMatrix = mainCamera.GetProjMatrix4x4();
			passConstants.viewMatrix = mainCamera.GetViewMatrix4x4();
			passConstants.shadowTransform = shadowMap.GetShadowTransform();
			passConstants.eyePos = glm::vec4(mainCamera.GetPosition3f(), 1.0f);
			passConstants.lights[0] = pointLight;
			passConstants.ambientLight = glm::vec4(0.3f, 0.3f, 0.3f, 0.3f);
			frameResource->passCB[0]->CopyData(&vkInfo.device, 0, 1, &passConstants);

			passConstants.projMatrix = shadowMap.GetLightProjMatrix();
			passConstants.viewMatrix = shadowMap.GetLightViewMatrix();
			frameResource->passCB[1]->CopyData(&vkInfo.device, 0, 1, &passConstants);

			//更新ObjectConstants
			for (uint32_t i = 0; i < ritems.size(); i++) {
				ObjectConstants objectConstants;
				objectConstants.worldMatrix = ritems[i]->worldMatrix;
				frameResource->objCB[ritems[i]->objCBIndex]->CopyData(&vkInfo.device, 0, 1, &objectConstants);
			}

			//17.4 Begin record commands
			if (vkInfo.cmd.begin(&cmdBeginInfo) != vk::Result::eSuccess) {
				MessageBox(0, L"Begin record command failed!!!", 0, 0);
			}

			//17.5 Wait for swap chain
			uint32_t currentBuffer = 0;
			if (vkInfo.device.acquireNextImageKHR(vkInfo.swapchain, UINT64_MAX, vkInfo.imageAcquiredSemaphore, vk::Fence(), &currentBuffer) != vk::Result::eSuccess) {
				MessageBox(0, L"Acquire next image failed!!!", 0, 0);
			}

			//绑定骨骼常量描述符
			vkInfo.cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo.pipelineLayout["scene"], 3, 1, &vkInfo.descSets[descCount + frameCount], 0, 0);

			//绘制阴影贴图
			shadowMap.BeginRenderPass(&vkInfo.cmd);
			vkInfo.cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo.pipelineLayout["scene"], 1, 1, &vkInfo.descSets[objectCount + 1], 0, 0);

			vkInfo.cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo.pipelines["shadow"]);
			DrawRenderItems(&vkInfo.cmd, vkInfo.pipelineLayout["scene"], vkInfo.descSets, renderLayers[(int)RenderLayer::opaque]);

			vkInfo.cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo.pipelines["skinnedShadow"]);
			DrawRenderItems(&vkInfo.cmd, vkInfo.pipelineLayout["scene"], vkInfo.descSets, renderLayers[(int)RenderLayer::skinned]);

			vkInfo.cmd.endRenderPass();

			//17.6 Begin render pass
			vk::ClearValue clearValue[2] = {
				vk::ClearColorValue(std::array<float, 4>({0.0f, 0.0f, 0.0f, 0.0f})),
				vk::ClearDepthStencilValue(1.0f, 0)
			};
			renderPassBeginInfo.setClearValueCount(2);
			renderPassBeginInfo.setPClearValues(clearValue);
			renderPassBeginInfo.setFramebuffer(vkInfo.scene.framebuffer);
			renderPassBeginInfo.setRenderPass(vkInfo.scenePass);
			vkInfo.cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
			
			//17.7 Bind Pipeline state
			vkInfo.cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo.pipelines["opaque"]);

			//绑定PassConstants描述符
			vkInfo.cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo.pipelineLayout["scene"], 1, 1, &vkInfo.descSets[objectCount], 0, 0);
			vkInfo.cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo.pipelineLayout["scene"], 2, 1, &vkInfo.descSets[descCount], 0, 0);

			DrawRenderItems(&vkInfo.cmd, vkInfo.pipelineLayout["scene"], vkInfo.descSets, renderLayers[(int)RenderLayer::opaque]);

			vkInfo.cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo.pipelines["skinned"]);
			DrawRenderItems(&vkInfo.cmd, vkInfo.pipelineLayout["scene"], vkInfo.descSets, renderLayers[(int)RenderLayer::skinned]);

			vkInfo.cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo.pipelines["skybox"]);
			DrawRenderItems(&vkInfo.cmd, vkInfo.pipelineLayout["scene"], vkInfo.descSets, renderLayers[(int)RenderLayer::skybox]);

			vkInfo.cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo.pipelines["smoke"]);
			vkInfo.cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo.pipelineLayout["scene"], 0, 1, &vkInfo.descSets[renderLayers[(int)RenderLayer::particle][1]->objCBIndex], 0, 0);
			flameParticle.DrawSubParticles(&vkInfo.cmd);
			vkInfo.cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo.pipelines["flame"]);
			vkInfo.cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo.pipelineLayout["scene"], 0, 1, &vkInfo.descSets[renderLayers[(int)RenderLayer::particle][0]->objCBIndex], 0, 0);
			flameParticle.DrawParticles(&vkInfo.cmd);

			vkInfo.cmd.endRenderPass();

			bloom.ExtractionBrightness(&vkInfo.cmd);
			bloom.BlurH(&vkInfo.cmd);
			bloom.BlurV(&vkInfo.cmd);

			/*完成图片融混*/
			renderPassBeginInfo.setClearValueCount(1);
			renderPassBeginInfo.setPClearValues(clearValue);
			renderPassBeginInfo.setFramebuffer(vkInfo.combineFramebuffers[currentBuffer]);
			renderPassBeginInfo.setRenderPass(vkInfo.combinePass);
			vkInfo.cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

			vkInfo.cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo.pipelines["combine"]);
			vkInfo.cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo.pipelineLayout["combine"], 0, 1, &bloom.GetSourceDescriptor(), 0, 0);
			vkInfo.cmd.draw(4, 1, 0, 0);
			vkInfo.cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo.pipelineLayout["combine"], 0, 1, &vkInfo.combinePassDescSets[0], 0, 0);
			vkInfo.cmd.draw(4, 1, 0, 0);

			vkInfo.cmd.endRenderPass();

			//17.10 Submit the command list
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
			
			//17.11 Wait for GPU to be finished4
			vk::Result waitingRes;
			do
				waitingRes = vkInfo.device.waitForFences(1, &vkInfo.fence, VK_TRUE, UINT64_MAX);
			while (waitingRes == vk::Result::eTimeout);

			//17.12 Present
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
		{
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