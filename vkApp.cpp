#include "vkApp.h"

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


void VkApp::Initialize(uint32_t windowWidth, uint32_t windowHeight, HWND hWnd, HINSTANCE hInstance) {
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

	/*Create instance*/

	//Application info
	vk::ApplicationInfo applicationInfo;
	applicationInfo.setApiVersion(VK_API_VERSION_1_0);
	applicationInfo.setPEngineName("Vulkan Graphics Engine");
	applicationInfo.setEngineVersion(1);
	applicationInfo.setPApplicationName("Base Project");
	applicationInfo.setApplicationVersion(1);

	//Create instance
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
	//Create debug messengr
	vk::DispatchLoaderDynamic dispatch(vkInfo.instance, GetInstanceProcAddr);
	if (vkInfo.instance.createDebugUtilsMessengerEXT(&debugMessengerInfo, 0, &vkInfo.debugMessenger, dispatch) != vk::Result::eSuccess) {
		MessageBox(0, L"Create debug messenger failed!!!", 0, 0);
	}
#endif

	/*Enumerate physical device*/
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

	/*Create device*/
	vk::DeviceCreateInfo deviceInfo;
	vk::DeviceQueueCreateInfo deviceQueueInfo;

	//Enumerate queue family properties
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
	if (!found)
		MessageBox(0, L"Cannot find graphics queue!!!", 0, 0);

	//Create device
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

	/*Create a swap chain*/

	//Create a surface fo win32
	const auto surfaceInfo = vk::Win32SurfaceCreateInfoKHR()
		.setHwnd(hWnd)
		.setHinstance(hInstance);
	if (vkInfo.instance.createWin32SurfaceKHR(&surfaceInfo, 0, &vkInfo.surface) != vk::Result::eSuccess) {
		MessageBox(0, L"Create Win32 surface failed!!!", 0, 0);
	}

	//Find present queue family
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
	if (!found)
		MessageBox(0, L"Do not support both graphics and present family!!!", 0, 0);

	//Create swap chain
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

	//Create frame buffer image views
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

	//Get queue
	vkInfo.device.getQueue(vkInfo.graphicsQueueFamilyIndex, 0, &vkInfo.queue);

	/*Create command buffers*/

	//Create command pool
	vk::CommandPoolCreateInfo commandPoolInfo;
	commandPoolInfo.setQueueFamilyIndex(vkInfo.graphicsQueueFamilyIndex);
	commandPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
	if (vkInfo.device.createCommandPool(&commandPoolInfo, 0, &vkInfo.cmdPool) != vk::Result::eSuccess) {
		MessageBox(0, L"Create command pool failed!!!", 0, 0);
	}

	//Allocate Command buffer from the pool
	vkInfo.cmd.resize(vkInfo.frameCount);
	vk::CommandBufferAllocateInfo cmdBufferAlloc;
	cmdBufferAlloc.setCommandPool(vkInfo.cmdPool);
	cmdBufferAlloc.setLevel(vk::CommandBufferLevel::ePrimary);
	cmdBufferAlloc.setCommandBufferCount(vkInfo.frameCount);
	if (vkInfo.device.allocateCommandBuffers(&cmdBufferAlloc, vkInfo.cmd.data()) != vk::Result::eSuccess) {
		MessageBox(0, L"Allocate command buffer failed!!!", 0, 0);
	}

	/*Prepare semaphore and fence*/
	auto semaphoreInfo = vk::SemaphoreCreateInfo();
	if (vkInfo.device.createSemaphore(&semaphoreInfo, 0, &vkInfo.imageAcquiredSemaphore) != vk::Result::eSuccess) {
		MessageBox(0, L"Create image acquired semaphore failed!!!", 0, 0);
	}

	auto fenceInfo = vk::FenceCreateInfo();
	if (vkInfo.device.createFence(&fenceInfo, 0, &vkInfo.fence) != vk::Result::eSuccess) {
		MessageBox(0, L"Create fence failed!!!", 0, 0);
	}

	PrepareScenePass();
	PrepareFinalPass();

	if (!vkInfo.input.Init(hInstance, hWnd)) {
		MessageBox(0, L"Init player input module failed!!!", 0, 0);
	}
}

void VkApp::PrepareScenePass() {
	/*Create image*/
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

	/*Create depth image*/
	vkInfo.depth.format = vk::Format::eD16Unorm;

	//Create image
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

	//Create image view
	auto depthImageViewInfo = vk::ImageViewCreateInfo()
		.setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA))
		.setFormat(vkInfo.depth.format)
		.setImage(vkInfo.depth.image)
		.setViewType(vk::ImageViewType::e2D)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
	vkInfo.device.createImageView(&depthImageViewInfo, 0, &vkInfo.depth.imageView);

	/*Create render pass*/
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

	/*Create frame buffer*/
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

void VkApp::PrepareFinalPass() {
	/*Create render pass*/
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

	vkInfo.device.createRenderPass(&renderPassInfo, 0, &vkInfo.finalPass);

	/*Create frame buffer*/
	vk::ImageView framebufferView;
	auto framebufferInfo = vk::FramebufferCreateInfo()
		.setRenderPass(vkInfo.finalPass)
		.setAttachmentCount(1)
		.setPAttachments(&framebufferView)
		.setWidth(vkInfo.width)
		.setHeight(vkInfo.height)
		.setLayers(1);

	vkInfo.finalFramebuffers.resize(vkInfo.frameCount);

	for (size_t i = 0; i < vkInfo.frameCount; i++) {
		framebufferView = vkInfo.swapchainImageViews[i];
		vkInfo.device.createFramebuffer(&framebufferInfo, 0, &vkInfo.finalFramebuffers[i]);
	}
}

void VkApp::Start() {
	//初始化场景系统
	scene.vkInfo = &vkInfo;

	mainCamera = Camera((float)vkInfo.width / (float)vkInfo.height);
	mainCamera.LookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	scene.SetAmbientLight(glm::vec4(0.3f, 0.3f, 0.3f, 0.3f));
	scene.SetMainCamera(&mainCamera);

	//创建一个点光
	scene.SetPointLight(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 10.0f);

	//利用Texture辅助库加载图片
	std::vector<std::unique_ptr<Texture>> textures;
	std::string texturePath[] = {
		"Assets\\brickTexture.jpg",
		"Assets\\icon.jpg",
		"Assets\\flame.png",
		"Assets\\SmokeLoop.png",
		"Assets\\brickTexture_normal.jpg"
	};
	for (size_t i = 0; i < 5; i++) {
		auto texture = std::make_unique<Texture>();
		LoadPixelWithSTB(texturePath[i].c_str(), 32, *texture, &vkInfo.device, vkInfo.gpu.getMemoryProperties());
		texture->SetupImage(&vkInfo.device, vkInfo.gpu.getMemoryProperties(), vkInfo.cmdPool, &vkInfo.queue);
		texture->CleanUploader(&vkInfo.device);
		textures.push_back(std::move(texture));
	}
	//加载立方体贴图
	Texture cubeMap;
	LoadCubeMapWithWIC(L"Assets\\skybox.png", GUID_WICPixelFormat32bppRGBA, cubeMap, &vkInfo.device, vkInfo.gpu.getMemoryProperties());
	cubeMap.SetupImage(&vkInfo.device, vkInfo.gpu.getMemoryProperties(), vkInfo.cmdPool, &vkInfo.queue);
	cubeMap.CleanUploader(&vkInfo.device);

	//创建用于光照的材质
	Material brick_mat;
	brick_mat.name = "brick";
	brick_mat.diffuse = textures[0].get();
	brick_mat.normal = textures[4].get();
	brick_mat.shaderModel = ShaderModel::normalMap;
	brick_mat.diffuseAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	brick_mat.fresnelR0 = glm::vec3(0.9f, 0.9f, 0.9f);
	brick_mat.roughness = 0.01f;
	brick_mat.matTransform = glm::mat4(1.0f);

	Material sphere_mat;
	sphere_mat.name = "sphere";
	sphere_mat.diffuse = textures[1].get();
	sphere_mat.diffuseAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	sphere_mat.fresnelR0 = glm::vec3(0.05f, 0.05f, 0.05f);
	sphere_mat.roughness = 0.8f;
	sphere_mat.matTransform = glm::mat4(1.0f);

	//将材质添加进场景中
	scene.AddMaterial(brick_mat);
	scene.AddMaterial(sphere_mat);

	//利用GameObject类创建场景中的物件
	GameObject plane_obj;
	plane_obj.name = "plane";
	plane_obj.material = scene.GetMaterial("brick");
	plane_obj.transform.position = glm::vec3(0.0f, -1.0f, 0.0f);

	GameObject sphere_obj;
	sphere_obj.name = "sphere";
	sphere_obj.material = scene.GetMaterial("sphere");
	sphere_obj.transform.position = glm::vec3(0.0f, 1.0f, 2.0f);

	//将物体添加进场景当中
	scene.AddGameObject(plane_obj, 0);
	scene.AddGameObject(sphere_obj, 0);

	//为物体添加渲染组件
	//使用GeometryGenerator库来辅助创建几何体
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData plane_mesh = geoGen.CreatePlane(30.0f, 30.0f, 10, 10);
	scene.AddMeshRenderer(scene.GetGameObject("plane"), plane_mesh.vertices, plane_mesh.indices);
	GeometryGenerator::MeshData sphere_mesh = geoGen.CreateGeosphere(0.5f, 8);
	scene.AddMeshRenderer(scene.GetGameObject("sphere"), sphere_mesh.vertices, sphere_mesh.indices);

	/*使用SkinnedModel类加载带有蒙皮动画的模型*/
	Model model("Assets\\model.fbx");

	//使用图片的文件名称作为GameObject的名称
	std::vector<std::string> meshNames;

	std::vector<std::unique_ptr<Texture>> modelTextures;
	for (size_t i = 0; i < model.texturePath.size(); i++) {
		auto texture = std::make_unique<Texture>();
		meshNames.push_back(model.texturePath[i].substr(model.texturePath[i].find_last_of('\\') + 1, model.texturePath[i].length() - 1));
		
		//使用STB库加载模型下的所有贴图并为其创建材质
		LoadPixelWithSTB(model.texturePath[i].c_str(), 32, *texture, &vkInfo.device, vkInfo.gpu.getMemoryProperties());
		texture->SetupImage(&vkInfo.device, vkInfo.gpu.getMemoryProperties(), vkInfo.cmdPool, &vkInfo.queue);
		texture->CleanUploader(&vkInfo.device);
		modelTextures.push_back(std::move(texture));

		Material material;
		material.name = meshNames[i];
		material.samplerType = SamplerType::border;
		material.diffuse = modelTextures[i].get();
		material.diffuseAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		material.fresnelR0 = glm::vec3(0.0f, 0.0f, 0.0f);
		material.matTransform = glm::mat4(1.0f);
		material.roughness = 0.8f;
		scene.AddMaterial(material);
	}

	//创建一个GameObject作为模型的父物件
	GameObject modelObject;
	modelObject.name = "marisaModel";
	modelObject.transform.position = glm::vec3(-2.0f, -1.0f, 0.0f);
	modelObject.transform.scale = glm::vec3(0.1f, 0.1f, 0.1f);
	modelObject.transform.localEulerAngle = glm::vec3(-glm::pi<float>() * 0.5f, 0.0f, 0.0f);
	scene.AddGameObject(modelObject, 0);

	//加载模型的所有的Mesh并添加到modelObject下
	for (size_t i = 0; i < model.renderInfo.size(); i++) {
		GameObject childObject;
		childObject.name = meshNames[i];
		childObject.material = scene.GetMaterial(meshNames[i]);
		scene.AddGameObject(childObject, scene.GetGameObject("marisaModel"));
		scene.AddMeshRenderer(scene.GetGameObject(meshNames[i]), model.renderInfo[i].vertices, model.renderInfo[i].indices);
	}

	/*初始化阴影贴图*/
	scene.SetShadowMap(vkInfo.width, vkInfo.height, glm::normalize(glm::vec3(-1.0f, 0.0f, 1.0f) - glm::vec3(1.0f, 1.0f, 0.0f)), 100.0f);

	/*初始化天空盒*/
	scene.SetSkybox(cubeMap, 0.5f, 8);

	/*创建粒子效果*/
	//创建粒子的材质(粒子不参与光照所以不需要指定光照参数)
	Material flame_mat;
	flame_mat.name = "flame";
	flame_mat.diffuse = textures[2].get();
	scene.AddMaterial(flame_mat);

	Material smoke_mat;
	smoke_mat.name = "smoke";
	smoke_mat.diffuse = textures[3].get();
	scene.AddMaterial(smoke_mat);

	//创建两个GameObject分别代表粒子和子粒子
	GameObject flameObject;
	flameObject.name = "flame";
	flameObject.material = scene.GetMaterial("flame");
	flameObject.transform.position = glm::vec3(1.0f, 1.0f, 0.0f);
	scene.AddGameObject(flameObject, 0);

	GameObject smokeObject;
	smokeObject.name = "smoke";
	smokeObject.material = scene.GetMaterial("smoke");
	smokeObject.transform.position = glm::vec3(0.0f, 1.0f, 0.0f);
	scene.AddGameObject(smokeObject, scene.GetGameObject("flame")); //设为flame的子物件

	//初始化火焰的粒子系统
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
	ParticleSystem::Emitter emitter;
	emitter.maxParticleNum = 10;
	emitter.position = glm::vec3(0.0f, 0.0f, 0.0f);
	emitter.radius = 0.3f;
	ParticleSystem::Texture particleTexture;
	particleTexture.splitX = 7;
	particleTexture.splitY = 7;
	particleTexture.texCount = 7 * 7;
	ParticleSystem::SubParticle subParticle;
	subParticle.color = glm::vec3(0.0f, 0.0f, 0.0f);
	subParticle.lastTime = 2.0f;
	subParticle.texture.splitX = 6;
	subParticle.texture.splitY = 6;
	subParticle.texture.texCount = 6 * 6;
	subParticle.size = 2.0f;
	subParticle.used = true;
	scene.AddParticleSystem(scene.GetGameObject("flame"), scene.GetGameObject("smoke"), property, emitter, particleTexture, subParticle);

	//设定后处理
	PostProcessingProfile::Bloom bloomProfile;
	bloomProfile.criticalValue = 0.5f;
	bloomProfile.blurOffset = 0.003f;
	bloomProfile.blurRadius = 5;
	scene.SetBloomPostProcessing(bloomProfile);

	//设定GUI
	scene.PrepareImGUI();

	scene.SetupVertexBuffer();
	scene.SetupDescriptors();
	scene.PreparePipeline();
	scene.PrepareShaderModel();
}

void VkApp::OnGUI() {
	ImGui::NewFrame();

	ImGui::SetNextWindowSize(ImVec2(300, 200), 0);
	ImGui::SetNextWindowPos(ImVec2(10, 10));
	ImGui::Begin("ImGUI Test");

	if (ImGui::Button("Particle System Switch", ImVec2(200, 30)))
		particleSystemEnabled = !particleSystemEnabled;

	ImGui::End();
	ImGui::Render();

	if (ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered())
		recordCommand = true;
}

void VkApp::Loop() {
	Update();
	OnGUI();

	scene.UpdateObjectConstants();
	scene.UpdatePassConstants();
	scene.UpdateMaterialConstants();
	scene.UpdateSkinnedModel(deltaTime);
	scene.UpdateImGUI(deltaTime);

	if (particleSystemEnabled)
		scene.UpdateCPUParticleSystem(deltaTime);

	//Wait for swap chain
	uint32_t currentBuffer;
	if (vkInfo.device.acquireNextImageKHR(vkInfo.swapchain, UINT64_MAX, vkInfo.imageAcquiredSemaphore, vk::Fence(), &currentBuffer) != vk::Result::eSuccess) {
		MessageBox(0, L"Acquire next image failed!!!", 0, 0);
	}
	
	//Record commands
	if (recordCommand) {
		auto cmdBeginInfo = vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

		for (uint32_t i = 0; i < vkInfo.frameCount; i++) {
			vkInfo.cmd[i].begin(&cmdBeginInfo);
			scene.DrawObject(vkInfo.cmd[i], i);
			vkInfo.cmd[i].end();
		}
		
		recordCommand = false;
	}

	//Submit the command list
	vk::PipelineStageFlags dstStageMask[] = {
		vk::PipelineStageFlagBits::eBottomOfPipe
	};

	vkInfo.device.resetFences(1, &vkInfo.fence);

	auto submitInfo = vk::SubmitInfo()
		.setCommandBufferCount(1)
		.setPCommandBuffers(&vkInfo.cmd[currentBuffer])
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&vkInfo.imageAcquiredSemaphore)
		.setPWaitDstStageMask(dstStageMask);
	vkInfo.queue.submit(1, &submitInfo, vkInfo.fence);

	//Wait for GPU to be finished
	vk::Result waitingRes;
	do
		waitingRes = vkInfo.device.waitForFences(1, &vkInfo.fence, VK_TRUE, UINT64_MAX);
	while (waitingRes == vk::Result::eTimeout);

	//Present
	auto presentInfo = vk::PresentInfoKHR()
		.setPImageIndices(&currentBuffer)
		.setSwapchainCount(1)
		.setPSwapchains(&vkInfo.swapchain);

	if (vkInfo.queue.presentKHR(&presentInfo) != vk::Result::eSuccess) {
		MessageBox(0, L"Present the render target failed!!!", 0, 0);
	}
}

void VkApp::Update() {
	//Update
	vkInfo.input.Update();
	scene.GetGameObject("sphere")->transform.localEulerAngle.y += glm::pi<float>() * deltaTime;
	scene.GetGameObject("sphere")->dirtyFlag = true;

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
}