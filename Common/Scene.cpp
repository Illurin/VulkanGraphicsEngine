#include "Scene.h"

void Scene::AddGameObject(GameObject& gameObject, GameObject* parent) {
	if (gameObjects.find(gameObject.name) != gameObjects.end()) {
		MessageBox(0, L"Cannot add the same material", 0, 0);
		return;
	}
	if(parent)
		gameObject.parent = parent;

	gameObjects[gameObject.name] = gameObject;
	if (!parent)
		rootObjects.push_back(&gameObjects[gameObject.name]);
	else
		parent->children.push_back(&gameObjects[gameObject.name]);
}

void Scene::AddMaterial(Material& material) {
	if (materials.find(material.name) != materials.end()) {
		MessageBox(0, L"Cannot add the same material", 0, 0);
		return;
	}
	materials[material.name] = material;
}

void Scene::AddMeshRenderer(GameObject* gameObject, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
	MeshRenderer meshRenderer;
	meshRenderer.vertices = vertices;
	meshRenderer.indices = indices;
	meshRenderer.gameObject = gameObject;
	meshRenderers.push_back(meshRenderer);
}

void Scene::AddSkinnedMeshRenderer(GameObject* gameObject, std::vector<SkinnedVertex>& vertices, std::vector<uint32_t>& indices) {
	SkinnedMeshRenderer meshRenderer;
	meshRenderer.vertices = vertices;
	meshRenderer.indices = indices;
	meshRenderer.gameObject = gameObject;
	if (skinnedModelInst.size() <= 0) {
		MessageBox(0, L"No skinned model can be used", 0, 0);
		return;
	}
	meshRenderer.skinnedModelIndex = skinnedModelInst.size() - 1;
	skinnedMeshRenderers.push_back(meshRenderer);
}

void Scene::AddSkinnedModelInstance(SkinnedModelInstance& skinnedModelInst) {
	this->skinnedModelInst.push_back(skinnedModelInst);
}

void Scene::AddParticleSystem(GameObject* particle, GameObject* subParticle, ParticleSystem::Property& property, ParticleSystem::Emitter& emitter, ParticleSystem::Texture& texture, ParticleSystem::SubParticle& subParticleProperty) {
	particleSystems.emplace_back(ParticleSystem());
	particleSystems.back().SetEmitterProperty(emitter);
	particleSystems.back().SetParticleProperty(property);
	particleSystems.back().SetTextureProperty(texture);
	particleSystems.back().SetSubParticle(subParticleProperty);
	particleSystems.back().particle = particle;
	particleSystems.back().subParticle = subParticle;
	particleSystems.back().PrepareParticles(&vkInfo->device, vkInfo->gpu.getMemoryProperties());
}

GameObject* Scene::GetGameObject(std::string name) {
	if (gameObjects.find(name) == gameObjects.end()) {
		MessageBox(0, L"Cannot find the game object", 0, 0);
		return 0;
	}
	return &gameObjects[name];
}

Material* Scene::GetMaterial(std::string name) {
	if (materials.find(name) == materials.end()) {
		MessageBox(0, L"Cannot find the material", 0, 0);
		return 0;
	}
	return &materials[name];
}

void Scene::SetAmbientLight(glm::vec3 strength) {
	ambientLight = strength;
}

void Scene::SetDirectionalLight(glm::vec3 direction, glm::vec3 strength) {
	directionalLight.direction = direction;
	directionalLight.strength = strength;
}

void Scene::SetPointLight(glm::vec3 position, glm::vec3 strength, float fallOffStart, float fallOffEnd) {
	pointLight.fallOffStart = fallOffStart;
	pointLight.fallOffEnd = fallOffEnd;
	pointLight.position = position;
	pointLight.strength = strength;
}

void Scene::SetSpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 strength, float fallOffStart, float fallOffEnd, float spotPower) {
	spotLight.direction = direction;
	spotLight.position = position;
	spotLight.strength = strength;
	spotLight.fallOffStart = fallOffStart;
	spotLight.fallOffEnd = fallOffEnd;
	spotLight.spotPower = spotPower;
}

void Scene::SetShadowMap(uint32_t width, uint32_t height, glm::vec3 lightDirection, float radius) {
	shadowMap.Init(&vkInfo->device, vkInfo->gpu.getMemoryProperties(), width, height);
	shadowMap.PrepareRenderPass(&vkInfo->device);
	shadowMap.PrepareFramebuffer(&vkInfo->device);
	shadowMap.SetLightTransformMatrix(lightDirection, radius);
}

void Scene::SetMainCamera(Camera* mainCamera) {
	this->mainCamera = mainCamera;
}

void Scene::SetSkybox(Texture image, float radius, uint32_t subdivision) {
	skybox.use = true;
	skybox.image = image;
	skybox.subdivision = subdivision;
	skybox.radius = radius;
}

void Scene::SetBloomPostProcessing(PostProcessingProfile::Bloom& profile) {
	bloom = std::make_unique<PostProcessing::Bloom>(profile);
}

void Scene::PrepareImGUI() {
	imgui = new ImGUI(vkInfo);
	imgui->Init(vkInfo->width, vkInfo->height);
	imgui->InitResource();
}

void Scene::UpdateImGUI(float deltaTime) {
	if (imgui) {
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)vkInfo->width, (float)vkInfo->height);
		io.DeltaTime = deltaTime;
		
		long cursorPosX;
		long cursorPosY;

		vkInfo->input.GetCursorPosition(cursorPosX, cursorPosY);

		io.MousePos = ImVec2((float)cursorPosX, (float)cursorPosY);
		io.MouseDown[0] = vkInfo->input.GetMouseDown(0);
		io.MouseDown[1] = vkInfo->input.GetMouseDown(1);
		
		imgui->UpdateBuffers();
	}
}

void Scene::CalcCildTransform(GameObject* gameObject, glm::mat4x4 parentToWorld) {
	if (gameObject->dirtyFlag) {
		glm::mat4x4 LR = glm::rotate(glm::mat4(1.0f), gameObject->transform.localEulerAngle.x, glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), gameObject->transform.localEulerAngle.y, glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), gameObject->transform.localEulerAngle.z, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4x4 S = glm::scale(glm::mat4(1.0f), glm::vec3(gameObject->transform.scale));
		glm::mat4x4 T = glm::translate(glm::mat4(1.0f), glm::vec3(gameObject->transform.position));
		glm::mat4x4 GR = glm::rotate(glm::mat4(1.0f), gameObject->transform.eulerAngle.x, glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), gameObject->transform.eulerAngle.y, glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), gameObject->transform.eulerAngle.z, glm::vec3(0.0f, 0.0f, 1.0f));
		gameObject->toParent = GR * T * S * LR;
		gameObject->dirtyFlag = false;
	}
	ObjectConstants objectConstants;
	objectConstants.worldMatrix = parentToWorld * gameObject->toParent;
	objectConstants.worldMatrix_trans_inv = glm::transpose(glm::inverse(objectConstants.worldMatrix));
	frameResources->objCB[gameObject->objCBIndex]->CopyData(&vkInfo->device, 0, 1, &objectConstants);
	for (auto& child : gameObject->children) {
		CalcCildTransform(child, objectConstants.worldMatrix);
	}
}

void Scene::UpdateObjectConstants() {
	for (auto& root : rootObjects) {
		CalcCildTransform(root, glm::mat4(1.0f));
	}
}

void Scene::UpdatePassConstants() {
	PassConstants passConstants;
	passConstants.projMatrix = shadowMap.GetLightProjMatrix();
	passConstants.viewMatrix = shadowMap.GetLightViewMatrix();
	frameResources->passCB[1]->CopyData(&vkInfo->device, 0, 1, &passConstants);
	passConstants.projMatrix = mainCamera->GetProjMatrix4x4();
	passConstants.viewMatrix = mainCamera->GetViewMatrix4x4();
	passConstants.eyePos = glm::vec4(mainCamera->GetPosition3f(), 1.0f);
	passConstants.shadowTransform = shadowMap.GetShadowTransform();
	passConstants.lights[0] = directionalLight;
	passConstants.lights[1] = pointLight;
	passConstants.lights[2] = spotLight;
	passConstants.ambientLight = glm::vec4(ambientLight, 1.0f);
	frameResources->passCB[0]->CopyData(&vkInfo->device, 0, 1, &passConstants);
}

void Scene::UpdateMaterialConstants() {
	for (auto& material : materials) {
		if (material.second.dirtyFlag) {
			MaterialConstants materialConstants;
			materialConstants.diffuseAlbedo = material.second.diffuseAlbedo;
			materialConstants.fresnelR0 = material.second.fresnelR0;
			materialConstants.matTransform = material.second.matTransform;
			materialConstants.roughness = material.second.roughness;
			frameResources->matCB[material.second.matCBIndex]->CopyData(&vkInfo->device, 0, 1, &materialConstants);
			material.second.dirtyFlag = false;
		}
	}
}

void Scene::UpdateSkinnedModel(float deltaTime) {
	for (auto& skinnedModel : skinnedModelInst) {
		skinnedModel.UpdateSkinnedAnimation(deltaTime);
		SkinnedConstants skinnedConstants;
		std::copy(std::begin(skinnedModel.finalTransforms), std::end(skinnedModel.finalTransforms), skinnedConstants.boneTransforms);

		for (uint32_t i = 0; i < skinnedModel.finalTransforms.size(); i++)
			skinnedConstants.boneTransforms_inv_trans[i] = glm::transpose(glm::inverse(skinnedConstants.boneTransforms[i]));

		frameResources->skinnedCB[skinnedModel.skinnedCBIndex]->CopyData(&vkInfo->device, 0, 1, &skinnedConstants);
	}
}

void Scene::UpdateCPUParticleSystem(float deltaTime) {
	for (auto& particleSystem : particleSystems) {
		particleSystem.UpdateParticles(deltaTime, &vkInfo->device);
	}
}

void Scene::SetupVertexBuffer() {
	std::vector<Vertex> vertices;
	std::vector<SkinnedVertex> skinnedVertices;
	std::vector<uint32_t> indices;

	for (auto& meshRenderer : meshRenderers) {
		meshRenderer.baseVertexLocation = vertices.size();
		meshRenderer.startIndexLocation = indices.size();
		vertices.insert(vertices.end(), meshRenderer.vertices.begin(), meshRenderer.vertices.end());
		indices.insert(indices.end(), meshRenderer.indices.begin(), meshRenderer.indices.end());
	}
	if (skybox.use) {
		GeometryGenerator geoGen;
		GeometryGenerator::MeshData skyboxMesh = geoGen.CreateGeosphere(skybox.radius, skybox.subdivision);
		skybox.baseVertexLocation = vertices.size();
		skybox.startIndexLocation = indices.size();
		skybox.indexCount = skyboxMesh.indices.size();
		vertices.insert(vertices.end(), skyboxMesh.vertices.begin(), skyboxMesh.vertices.end());
		indices.insert(indices.end(), skyboxMesh.indices.begin(), skyboxMesh.indices.end());
	}
	for (auto& meshRenderer : skinnedMeshRenderers) {
		meshRenderer.baseVertexLocation = skinnedVertices.size();
		meshRenderer.startIndexLocation = indices.size();
		skinnedVertices.insert(skinnedVertices.end(), meshRenderer.vertices.begin(), meshRenderer.vertices.end());
		indices.insert(indices.end(), meshRenderer.indices.begin(), meshRenderer.indices.end());
	}

	vk::MemoryPropertyFlags memProp = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
	vk::PhysicalDeviceMemoryProperties gpuProp = vkInfo->gpu.getMemoryProperties();

	if (vertexBuffer != nullptr)
		vertexBuffer->DestroyBuffer(&vkInfo->device);
	vertexBuffer = std::make_unique<Buffer<Vertex>>(&vkInfo->device, vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer, gpuProp, memProp, false);
	vertexBuffer->CopyData(&vkInfo->device, 0, vertices.size(), vertices.data());

	if (skinnedMeshRenderers.size() > 0) {
		if (skinnedVertexBuffer != nullptr)
			skinnedVertexBuffer->DestroyBuffer(&vkInfo->device);
		skinnedVertexBuffer = std::make_unique<Buffer<SkinnedVertex>>(&vkInfo->device, skinnedVertices.size(), vk::BufferUsageFlagBits::eVertexBuffer, gpuProp, memProp, false);
		skinnedVertexBuffer->CopyData(&vkInfo->device, 0, skinnedVertices.size(), skinnedVertices.data());
	}

	if (indexBuffer != nullptr)
		indexBuffer->DestroyBuffer(&vkInfo->device);
	indexBuffer = std::make_unique<Buffer<uint32_t>>(&vkInfo->device, indices.size(), vk::BufferUsageFlagBits::eIndexBuffer, gpuProp, memProp, false);
	indexBuffer->CopyData(&vkInfo->device, 0, indices.size(), indices.data());
}

void Scene::SetupDescriptors() {
	//初始化FrameBuffer
	frameResources = std::make_unique<FrameResource>(&vkInfo->device, vkInfo->gpu.getMemoryProperties(), 2, gameObjects.size(), materials.size(), skinnedModelInst.size());
	
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
		vkInfo->device.createSampler(&samplerInfo, 0, &repeatSampler);

		samplerInfo.setAddressModeU(vk::SamplerAddressMode::eClampToBorder);
		samplerInfo.setAddressModeV(vk::SamplerAddressMode::eClampToBorder);
		samplerInfo.setAddressModeW(vk::SamplerAddressMode::eClampToBorder);
		vkInfo->device.createSampler(&samplerInfo, 0, &borderSampler);
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
		vkInfo->device.createSampler(&samplerInfo, 0, &comparisonSampler);
	}

	//第一个管线布局：世界矩阵
	auto objCBBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	vk::DescriptorSetLayoutBinding layoutBindingObject[] = {
		objCBBinding
	};

	//第二个管线布局：纹理，法线贴图，材质常量和采样器
	auto materialCBBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto samplerBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto textureBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(2)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampledImage)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto normalMapBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(3)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampledImage)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	vk::DescriptorSetLayoutBinding layoutBindingMaterial[] = {
		textureBinding, materialCBBinding, samplerBinding, normalMapBinding
	};

	//第三个管线布局：Pass常量
	auto passCBBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry | vk::ShaderStageFlagBits::eFragment);

	vk::DescriptorSetLayoutBinding layoutBindingPass[] = {
		passCBBinding
	};

	//第四个管线布局：比较采样器和阴影贴图
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

	//第五个管线布局：骨骼的变换矩阵
	auto layoutBindingSkinned = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	//后处理效果的管线布局：采样器和源贴图
	auto finalSamplerBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto finalImageBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampledImage)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	vk::DescriptorSetLayoutBinding layoutBindingFinal[] = {
		finalSamplerBinding, finalImageBinding
	};

	//为渲染管线提供管线布局
	vkInfo->descSetLayout.resize(5);

	auto descLayoutInfo_obj = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(1)
		.setPBindings(layoutBindingObject);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_obj, 0, &vkInfo->descSetLayout[0]);

	auto descLayoutInfo_material = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(4)
		.setPBindings(layoutBindingMaterial);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_material, 0, &vkInfo->descSetLayout[1]);

	auto descLayoutInfo_pass = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(1)
		.setPBindings(layoutBindingPass);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_pass, 0, &vkInfo->descSetLayout[2]);

	auto descLayoutInfo_shadow = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(2)
		.setPBindings(layoutBindingShadow);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_shadow, 0, &vkInfo->descSetLayout[3]);

	auto descLayoutInfo_skinned = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(1)
		.setPBindings(&layoutBindingSkinned);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_skinned, 0, &vkInfo->descSetLayout[4]);

	//为finalPass提供管线布局
	auto descLayoutInfo_final = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(2)
		.setPBindings(layoutBindingFinal);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_final, 0, &vkInfo->finalPassLayout);

	//为描述符的分配提供布局
	uint32_t objCount = gameObjects.size();
	uint32_t matCount = materials.size();
	uint32_t descCount = objCount + matCount + passCount + skinnedModelInst.size() + 1;

	//创建描述符池
	uint32_t postprocessingDescCount = bloom ? bloom->GetRequiredDescCount() : 0;

	vk::DescriptorPoolSize typeCount[3];
	typeCount[0].setType(vk::DescriptorType::eUniformBuffer);
	typeCount[0].setDescriptorCount(matCount + objCount + passCount + skinnedModelInst.size() + (skybox.use ? 1 : 0));
	typeCount[1].setType(vk::DescriptorType::eSampledImage);
	typeCount[1].setDescriptorCount(matCount * 2 + 2 + 1 + (skybox.use ? 1 : 0) + postprocessingDescCount);
	typeCount[2].setType(vk::DescriptorType::eSampler);
	typeCount[2].setDescriptorCount(matCount + 1 + 1 + (skybox.use ? 1 : 0) + postprocessingDescCount);

	auto descriptorPoolInfo = vk::DescriptorPoolCreateInfo()
		.setMaxSets(descCount + 1 + (skybox.use ? 1 : 0) + postprocessingDescCount)
		.setPoolSizeCount(3)
		.setPPoolSizes(typeCount);
	vkInfo->device.createDescriptorPool(&descriptorPoolInfo, 0, &vkInfo->descPool);

	//分配描述符
	vk::DescriptorSetAllocateInfo descSetAllocInfo;

	for (auto& gameObject : gameObjects) {
		descSetAllocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(vkInfo->descPool)
			.setDescriptorSetCount(1)
			.setPSetLayouts(&vkInfo->descSetLayout[0]);
		vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, &gameObject.second.descSet);
	}

	for (auto& material : materials) {
		descSetAllocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(vkInfo->descPool)
			.setDescriptorSetCount(1)
			.setPSetLayouts(&vkInfo->descSetLayout[1]);
		vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, &material.second.descSet);
	}

	descSetAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(vkInfo->descPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&vkInfo->descSetLayout[2]);
	vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, &scenePassDesc);

	descSetAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(vkInfo->descPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&vkInfo->descSetLayout[2]);
	vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, &shadowPassDesc);

	descSetAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(vkInfo->descPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&vkInfo->descSetLayout[3]);
	vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, &drawShadowDesc);

	//分配蒙皮动画的描述符
	for (auto& skinnedModel : skinnedModelInst) {
		descSetAllocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(vkInfo->descPool)
			.setDescriptorSetCount(1)
			.setPSetLayouts(&vkInfo->descSetLayout[4]);
		vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, &skinnedModel.descSet);
	}

	//分配天空盒的描述符
	if (skybox.use) {
		descSetAllocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(vkInfo->descPool)
			.setDescriptorSetCount(1)
			.setPSetLayouts(&vkInfo->descSetLayout[1]);
		vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, &skybox.descSet);
	}

	//为finalPass分配一个描述符
	vkInfo->finalPassDescSets.resize(1);

	descSetAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(vkInfo->descPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&vkInfo->finalPassLayout);
	vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, vkInfo->finalPassDescSets.data());

	//更新每一个描述符
	uint32_t objCBIndex = 0;
	for (auto& gameObject : gameObjects) {
		gameObject.second.objCBIndex = objCBIndex;

		auto descriptorObjCBInfo = vk::DescriptorBufferInfo()
			.setBuffer(frameResources->objCB[objCBIndex]->GetBuffer())
			.setOffset(0)
			.setRange(sizeof(ObjectConstants));

		vk::WriteDescriptorSet descSetWrites[1];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(0);
		descSetWrites[0].setDstSet(gameObject.second.descSet);
		descSetWrites[0].setPBufferInfo(&descriptorObjCBInfo);
		vkInfo->device.updateDescriptorSets(1, descSetWrites, 0, 0);

		objCBIndex++;
	}

	uint32_t matCBIndex = 0;
	for (auto& material : materials) {
		material.second.matCBIndex = matCBIndex;

		auto descriptorMatCBInfo = vk::DescriptorBufferInfo()
			.setBuffer(frameResources->matCB[matCBIndex]->GetBuffer())
			.setOffset(0)
			.setRange(sizeof(MaterialConstants));

		vk::DescriptorImageInfo descriptorSamplerInfo;
		if (material.second.samplerType == SamplerType::repeat)
			descriptorSamplerInfo.setSampler(repeatSampler);
		else if (material.second.samplerType == SamplerType::border)
			descriptorSamplerInfo.setSampler(borderSampler);

		auto descriptorDiffuseInfo = vk::DescriptorImageInfo()
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(material.second.diffuse->GetImageView(&vkInfo->device));

		vk::WriteDescriptorSet descSetWrites[4];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(0);
		descSetWrites[0].setDstSet(material.second.descSet);
		descSetWrites[0].setPBufferInfo(&descriptorMatCBInfo);
		descSetWrites[1].setDescriptorCount(1);
		descSetWrites[1].setDescriptorType(vk::DescriptorType::eSampler);
		descSetWrites[1].setDstArrayElement(0);
		descSetWrites[1].setDstBinding(1);
		descSetWrites[1].setDstSet(material.second.descSet);
		descSetWrites[1].setPImageInfo(&descriptorSamplerInfo);
		descSetWrites[2].setDescriptorCount(1);
		descSetWrites[2].setDescriptorType(vk::DescriptorType::eSampledImage);
		descSetWrites[2].setDstArrayElement(0);
		descSetWrites[2].setDstBinding(2);
		descSetWrites[2].setDstSet(material.second.descSet);
		descSetWrites[2].setPImageInfo(&descriptorDiffuseInfo);

		int updateCount = 3;

		if (material.second.shaderModel == ShaderModel::normalMap) {
			auto descriptorNormalInfo = vk::DescriptorImageInfo()
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImageView(material.second.normal->GetImageView(&vkInfo->device));
		
			descSetWrites[3].setDescriptorCount(1);
			descSetWrites[3].setDescriptorType(vk::DescriptorType::eSampledImage);
			descSetWrites[3].setDstArrayElement(0);
			descSetWrites[3].setDstBinding(3);
			descSetWrites[3].setDstSet(material.second.descSet);
			descSetWrites[3].setPImageInfo(&descriptorNormalInfo);

			updateCount++;
		}

		vkInfo->device.updateDescriptorSets(updateCount, descSetWrites, 0, 0);

		matCBIndex++;
	}

	{
		auto descriptrorPassCBInfo = vk::DescriptorBufferInfo()
			.setBuffer(frameResources->passCB[0]->GetBuffer())
			.setOffset(0)
			.setRange(sizeof(PassConstants));

		vk::WriteDescriptorSet descSetWrites[1];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(0);
		descSetWrites[0].setDstSet(scenePassDesc);
		descSetWrites[0].setPBufferInfo(&descriptrorPassCBInfo);
		vkInfo->device.updateDescriptorSets(1, descSetWrites, 0, 0);
	}
	{
		auto descriptrorPassCBInfo = vk::DescriptorBufferInfo()
			.setBuffer(frameResources->passCB[1]->GetBuffer())
			.setOffset(0)
			.setRange(sizeof(PassConstants));

		vk::WriteDescriptorSet descSetWrites[1];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(0);
		descSetWrites[0].setDstSet(shadowPassDesc);
		descSetWrites[0].setPBufferInfo(&descriptrorPassCBInfo);
		vkInfo->device.updateDescriptorSets(1, descSetWrites, 0, 0);
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
		descSetWrites[0].setDstSet(drawShadowDesc);
		descSetWrites[0].setPImageInfo(&descriptorSamplerInfo);
		descSetWrites[1].setDescriptorCount(1);
		descSetWrites[1].setDescriptorType(vk::DescriptorType::eSampledImage);
		descSetWrites[1].setDstArrayElement(0);
		descSetWrites[1].setDstBinding(1);
		descSetWrites[1].setDstSet(drawShadowDesc);
		descSetWrites[1].setPImageInfo(&descriptorShadowMapInfo);
		vkInfo->device.updateDescriptorSets(2, descSetWrites, 0, 0);
	}

	//更新蒙皮动画的描述符
	uint32_t skinnedCBIndex = 0;
	for(auto& skinnedModel : skinnedModelInst) {
		auto descriptrorSkinnedCBInfo = vk::DescriptorBufferInfo()
			.setBuffer(frameResources->skinnedCB[skinnedCBIndex]->GetBuffer())
			.setOffset(0)
			.setRange(sizeof(SkinnedConstants));

		vk::WriteDescriptorSet descSetWrites[1];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(0);
		descSetWrites[0].setDstSet(skinnedModel.descSet);
		descSetWrites[0].setPBufferInfo(&descriptrorSkinnedCBInfo);
		vkInfo->device.updateDescriptorSets(1, descSetWrites, 0, 0);
		
		skinnedModel.skinnedCBIndex = skinnedCBIndex;
		skinnedCBIndex++;
	}

	//更新天空盒的描述符
	if(skybox.use) {
		auto descriptorSamplerInfo = vk::DescriptorImageInfo()
			.setSampler(repeatSampler);

		auto descriptorImageInfo = vk::DescriptorImageInfo()
			.setImageView(skybox.image.GetImageView(&vkInfo->device))
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::WriteDescriptorSet descSetWrites[2];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eSampler);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(1);
		descSetWrites[0].setDstSet(skybox.descSet);
		descSetWrites[0].setPImageInfo(&descriptorSamplerInfo);
		descSetWrites[1].setDescriptorCount(1);
		descSetWrites[1].setDescriptorType(vk::DescriptorType::eSampledImage);
		descSetWrites[1].setDstArrayElement(0);
		descSetWrites[1].setDstBinding(2);
		descSetWrites[1].setDstSet(skybox.descSet);
		descSetWrites[1].setPImageInfo(&descriptorImageInfo);
		vkInfo->device.updateDescriptorSets(2, descSetWrites, 0, 0);
	}

	//初始化后处理
	if (bloom) bloom->Init(vkInfo, vkInfo->scene.imageView);

	//更新FinalPass的描述符
	{
		auto descriptorSamplerInfo = vk::DescriptorImageInfo()
			.setSampler(repeatSampler);
		
		auto descriptorImageInfo = vk::DescriptorImageInfo()
			.setImageView((bloom) ? (bloom->GetImageView()) : (vkInfo->scene.imageView))
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::WriteDescriptorSet descSetWrites[2];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eSampler);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(0);
		descSetWrites[0].setDstSet(vkInfo->finalPassDescSets[0]);
		descSetWrites[0].setPImageInfo(&descriptorSamplerInfo);
		descSetWrites[1].setDescriptorCount(1);
		descSetWrites[1].setDescriptorType(vk::DescriptorType::eSampledImage);
		descSetWrites[1].setDstArrayElement(0);
		descSetWrites[1].setDstBinding(1);
		descSetWrites[1].setDstSet(vkInfo->finalPassDescSets[0]);
		descSetWrites[1].setPImageInfo(&descriptorImageInfo);
		vkInfo->device.updateDescriptorSets(2, descSetWrites, 0, 0);
	}
}

void Scene::PreparePipeline() {
	//顶点输入装配属性
	vkInfo->vertex.binding.setBinding(0);
	vkInfo->vertex.binding.setInputRate(vk::VertexInputRate::eVertex);
	vkInfo->vertex.binding.setStride(sizeof(Vertex));

	vkInfo->vertex.attrib.resize(4);

	vkInfo->vertex.attrib[0].setBinding(0);
	vkInfo->vertex.attrib[0].setFormat(vk::Format::eR32G32B32Sfloat);
	vkInfo->vertex.attrib[0].setLocation(0);
	vkInfo->vertex.attrib[0].setOffset(0);

	vkInfo->vertex.attrib[1].setBinding(0);
	vkInfo->vertex.attrib[1].setFormat(vk::Format::eR32G32Sfloat);
	vkInfo->vertex.attrib[1].setLocation(1);
	vkInfo->vertex.attrib[1].setOffset(sizeof(glm::vec3));

	vkInfo->vertex.attrib[2].setBinding(0);
	vkInfo->vertex.attrib[2].setFormat(vk::Format::eR32G32B32Sfloat);
	vkInfo->vertex.attrib[2].setLocation(2);
	vkInfo->vertex.attrib[2].setOffset(sizeof(glm::vec3) + sizeof(glm::vec2));

	vkInfo->vertex.attrib[3].setBinding(0);
	vkInfo->vertex.attrib[3].setFormat(vk::Format::eR32G32B32Sfloat);
	vkInfo->vertex.attrib[3].setLocation(3);
	vkInfo->vertex.attrib[3].setOffset(2 * sizeof(glm::vec3) + sizeof(glm::vec2));

	//蒙皮网格的顶点输入装配属性
	vk::VertexInputBindingDescription skinnedBinding;
	std::vector<vk::VertexInputAttributeDescription> skinnedAttrib;

	skinnedBinding.setBinding(0);
	skinnedBinding.setInputRate(vk::VertexInputRate::eVertex);
	skinnedBinding.setStride(sizeof(SkinnedVertex));

	skinnedAttrib.resize(6);

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
	skinnedAttrib[4].setFormat(vk::Format::eR32G32B32Sfloat);
	skinnedAttrib[4].setLocation(4);
	skinnedAttrib[4].setOffset(3 * sizeof(glm::vec3) + sizeof(glm::vec2));

	skinnedAttrib[5].setBinding(0);
	skinnedAttrib[5].setFormat(vk::Format::eR32G32B32A32Uint);
	skinnedAttrib[5].setLocation(5);
	skinnedAttrib[5].setOffset(4 * sizeof(glm::vec3) + sizeof(glm::vec2));

	/*Create pipelines*/
	auto vsModule = CreateShaderModule("Shaders\\vertex.spv", vkInfo->device);
	auto psModule = CreateShaderModule("Shaders\\fragment.spv", vkInfo->device);

	//Create pipeline shader module
	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderInfo(2);

	pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(vsModule)
		.setStage(vk::ShaderStageFlagBits::eVertex);

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
		.setDepthTestEnable(VK_TRUE)
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

	//Pipeline layout
	auto plInfo = vk::PipelineLayoutCreateInfo()
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(0)
		.setSetLayoutCount(vkInfo->descSetLayout.size())
		.setPSetLayouts(vkInfo->descSetLayout.data());
	vkInfo->device.createPipelineLayout(&plInfo, 0, &vkInfo->pipelineLayout["scene"]);

	auto shaderModelSME = vk::SpecializationMapEntry()
		.setConstantID(0)
		.setOffset(0)
		.setSize(sizeof(int));

	for (int i = 0; i < (int)ShaderModel::shaderModelCount; i++) {
		auto shaderModelSI = vk::SpecializationInfo()
			.setDataSize(sizeof(int))
			.setMapEntryCount(1)
			.setPMapEntries(&shaderModelSME)
			.setPData(&i);

		pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
			.setPName("main")
			.setModule(psModule)
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setPSpecializationInfo(&shaderModelSI);

		meshPipeline.push_back(CreateGraphicsPipeline(vkInfo->device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo->pipelineLayout["scene"], pipelineShaderInfo, vkInfo->scenePass));
	}
	vkInfo->device.destroyShaderModule(vsModule);

	//编译用于蒙皮动画的着色器
	vsModule = CreateShaderModule("Shaders\\skinnedVS.spv", vkInfo->device);

	pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(vsModule)
		.setStage(vk::ShaderStageFlagBits::eVertex);

	//创建用于蒙皮动画的管线
	auto skinnedviInfo = viInfo;
	skinnedviInfo.setVertexBindingDescriptionCount(1);
	skinnedviInfo.setPVertexBindingDescriptions(&skinnedBinding);
	skinnedviInfo.setVertexAttributeDescriptionCount(skinnedAttrib.size());
	skinnedviInfo.setPVertexAttributeDescriptions(skinnedAttrib.data());

	for (int i = 0; i < (int)ShaderModel::shaderModelCount; i++) {
		auto shaderModelSI = vk::SpecializationInfo()
			.setDataSize(sizeof(int))
			.setMapEntryCount(1)
			.setPMapEntries(&shaderModelSME)
			.setPData(&i);

		pipelineShaderInfo[1] = vk::PipelineShaderStageCreateInfo()
			.setPName("main")
			.setModule(psModule)
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setPSpecializationInfo(&shaderModelSI);

		skinnedMeshPipeline.push_back(CreateGraphicsPipeline(vkInfo->device, dynamicInfo, skinnedviInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo->pipelineLayout["scene"], pipelineShaderInfo, vkInfo->scenePass));
	}
	vkInfo->device.destroyShaderModule(vsModule);
	vkInfo->device.destroyShaderModule(psModule);

	//编译用于天空球的着色器
	vsModule = CreateShaderModule("Shaders\\skyboxVS.spv", vkInfo->device);
	psModule = CreateShaderModule("Shaders\\skyboxPS.spv", vkInfo->device);

	pipelineShaderInfo[0].setModule(vsModule);
	pipelineShaderInfo[1].setModule(psModule);

	//创建用于绘制天空球的管线
	dsInfo.setDepthCompareOp(vk::CompareOp::eLessOrEqual);

	vk::DescriptorSetLayout descSetLayout[] = { vkInfo->descSetLayout[1], vkInfo->descSetLayout[2] };

	auto skyboxPipelineInfo = vk::PipelineLayoutCreateInfo()
		.setSetLayoutCount(2)
		.setPSetLayouts(descSetLayout);
	vkInfo->device.createPipelineLayout(&skyboxPipelineInfo, 0, &vkInfo->pipelineLayout["skybox"]);
	vkInfo->pipelines["skybox"] = CreateGraphicsPipeline(vkInfo->device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo->pipelineLayout["skybox"], pipelineShaderInfo, vkInfo->scenePass);

	vkInfo->device.destroyShaderModule(vsModule);
	vkInfo->device.destroyShaderModule(psModule);

	//编译用于阴影贴图的着色器
	vsModule = CreateShaderModule("Shaders\\shadowVS.spv", vkInfo->device);
	psModule = CreateShaderModule("Shaders\\shadowPS.spv", vkInfo->device);

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

	vkInfo->pipelines["shadow"] = CreateGraphicsPipeline(vkInfo->device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo->pipelineLayout["scene"], pipelineShaderInfo, shadowMap.GetRenderPass());
	vkInfo->device.destroyShaderModule(vsModule);

	//编译用于蒙皮动画的阴影着色器
	vsModule = CreateShaderModule("Shaders\\shadowSkinnedVS.spv", vkInfo->device);
	pipelineShaderInfo[0] = vk::PipelineShaderStageCreateInfo()
		.setPName("main")
		.setModule(vsModule)
		.setStage(vk::ShaderStageFlagBits::eVertex);

	//创建用于蒙皮动画的阴影管线
	vkInfo->pipelines["skinnedShadow"] = CreateGraphicsPipeline(vkInfo->device, dynamicInfo, skinnedviInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo->pipelineLayout["scene"], pipelineShaderInfo, shadowMap.GetRenderPass());
	vkInfo->device.destroyShaderModule(psModule);

	//编译用于图像混合的着色器
	vsModule = CreateShaderModule("Shaders\\bloomVS.spv", vkInfo->device);
	psModule = CreateShaderModule("Shaders\\combine.spv", vkInfo->device);

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
		.setPSetLayouts(&vkInfo->finalPassLayout);
	vkInfo->device.createPipelineLayout(&combinePipelineLayoutInfo, 0, &vkInfo->pipelineLayout["final"]);

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

	vkInfo->pipelines["final"] = CreateGraphicsPipeline(vkInfo->device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo->pipelineLayout["final"], pipelineShaderInfo, vkInfo->finalPass);

	vkInfo->device.destroyShaderModule(vsModule);
	vkInfo->device.destroyShaderModule(psModule);

	//粒子的顶点输入装配属性
	vk::VertexInputBindingDescription particleBinding;
	std::vector<vk::VertexInputAttributeDescription> particleAttrib;

	particleBinding.setBinding(0);
	particleBinding.setInputRate(vk::VertexInputRate::eVertex);
	particleBinding.setStride(sizeof(ParticleSystem::Particle));

	particleAttrib.resize(4);

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

	//编译用于粒子效果的着色器
	vsModule = CreateShaderModule("Shaders\\particleVS.spv", vkInfo->device);
	vk::ShaderModule gsModule = CreateShaderModule("Shaders\\particleGS.spv", vkInfo->device);
	psModule = CreateShaderModule("Shaders\\particlePS.spv", vkInfo->device);

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
		.setVertexAttributeDescriptionCount(particleAttrib.size())
		.setPVertexAttributeDescriptions(particleAttrib.data());

	vkInfo->pipelines["smoke"] = CreateGraphicsPipeline(vkInfo->device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo->pipelineLayout["scene"], pipelineShaderInfo, vkInfo->scenePass);

	attState.setDstColorBlendFactor(vk::BlendFactor::eOne);

	vkInfo->pipelines["flame"] = CreateGraphicsPipeline(vkInfo->device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo->pipelineLayout["scene"], pipelineShaderInfo, vkInfo->scenePass);

	vkInfo->device.destroyShaderModule(vsModule);
	vkInfo->device.destroyShaderModule(gsModule);
	vkInfo->device.destroyShaderModule(psModule);
}

void Scene::PrepareShaderModel() {
	for (auto& meshRenderer : meshRenderers) {
		shaderModel[(int)meshRenderer.gameObject->material->shaderModel].push_back(&meshRenderer);
	}
	for (auto& skinnedMeshRenderer : skinnedMeshRenderers) {
		skinnedShaderModel[(int)skinnedMeshRenderer.gameObject->material->shaderModel].push_back(&skinnedMeshRenderer);
	}
}

void Scene::DrawObject(vk::CommandBuffer cmd, uint32_t currentBuffer) {
	shadowMap.BeginRenderPass(&cmd);

	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 2, 1, &shadowPassDesc, 0, 0);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo->pipelines["shadow"]);
	
	vk::DeviceSize offsets[] = { 0 };
	cmd.bindIndexBuffer(indexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
	cmd.bindVertexBuffers(0, 1, &vertexBuffer->GetBuffer(), offsets);

	for (auto& meshRenderer : meshRenderers) {
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 0, 1, &meshRenderer.gameObject->descSet, 0, 0);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 1, 1, &meshRenderer.gameObject->material->descSet, 0, 0);
		cmd.drawIndexed(meshRenderer.indices.size(), 1, meshRenderer.startIndexLocation, meshRenderer.baseVertexLocation, 1);
	}
	if (skinnedModelInst.size() > 0) {
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo->pipelines["skinnedShadow"]);
		cmd.bindVertexBuffers(0, 1, &skinnedVertexBuffer->GetBuffer(), offsets);
		for (auto& skinnedMeshRenderer : skinnedMeshRenderers) {
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 0, 1, &skinnedMeshRenderer.gameObject->descSet, 0, 0);
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 4, 1, &skinnedModelInst[skinnedMeshRenderer.skinnedModelIndex].descSet, 0, 0);
			cmd.drawIndexed(skinnedMeshRenderer.indices.size(), 1, skinnedMeshRenderer.startIndexLocation, skinnedMeshRenderer.baseVertexLocation, 1);
		}
	}

	cmd.endRenderPass();

	//Begin render pass
	vk::RenderPassBeginInfo renderPassBeginInfo;
	vk::ClearValue clearValue[2] = {
		vk::ClearColorValue(std::array<float, 4>({0.0f, 0.0f, 0.0f, 0.0f})),
		vk::ClearDepthStencilValue(1.0f, 0)
	};
	renderPassBeginInfo.setClearValueCount(2);
	renderPassBeginInfo.setPClearValues(clearValue);
	renderPassBeginInfo.setFramebuffer(vkInfo->scene.framebuffer);
	renderPassBeginInfo.setRenderPass(vkInfo->scenePass);
	renderPassBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(0.0f, 0.0f), vk::Extent2D(vkInfo->width, vkInfo->height)));
	cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 2, 1, &scenePassDesc, 0, 0);
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 3, 1, &drawShadowDesc, 0, 0);

	cmd.bindVertexBuffers(0, 1, &vertexBuffer->GetBuffer(), offsets);

	for (int i = 0; i < (int)ShaderModel::shaderModelCount; i++) {
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, meshPipeline[i]);
		for (auto& meshRenderer : shaderModel[i]) {
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 0, 1, &meshRenderer->gameObject->descSet, 0, 0);
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 1, 1, &meshRenderer->gameObject->material->descSet, 0, 0);
			cmd.drawIndexed(meshRenderer->indices.size(), 1, meshRenderer->startIndexLocation, meshRenderer->baseVertexLocation, 1);
		}
	}
	if (skinnedModelInst.size() > 0) {
		cmd.bindVertexBuffers(0, 1, &skinnedVertexBuffer->GetBuffer(), offsets);
		for (int i = 0; i < (int)ShaderModel::shaderModelCount; i++) {
			cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, skinnedMeshPipeline[i]);
			for (auto& skinnedMeshRenderer : skinnedShaderModel[i]) {
				cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 0, 1, &skinnedMeshRenderer->gameObject->descSet, 0, 0);
				cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 1, 1, &skinnedMeshRenderer->gameObject->material->descSet, 0, 0);
				cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 4, 1, &skinnedModelInst[skinnedMeshRenderer->skinnedModelIndex].descSet, 0, 0);
				cmd.drawIndexed(skinnedMeshRenderer->indices.size(), 1, skinnedMeshRenderer->startIndexLocation, skinnedMeshRenderer->baseVertexLocation, 1);
			}
		}
	}

	//绘制天空盒
	cmd.bindVertexBuffers(0, 1, &vertexBuffer->GetBuffer(), offsets);

	if (skybox.use) {
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo->pipelines["skybox"]);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["skybox"], 0, 1, &skybox.descSet, 0, 0);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["skybox"], 1, 1, &scenePassDesc, 0, 0);
		cmd.drawIndexed(skybox.indexCount, 1, skybox.startIndexLocation, skybox.baseVertexLocation, 1);
	}

	//绘制粒子系统
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 2, 1, &scenePassDesc, 0, 0);
	cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 3, 1, &drawShadowDesc, 0, 0);

	for (auto& particleSystem : particleSystems) {
		if (particleSystem.subParticle) {
			cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo->pipelines["smoke"]);
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 0, 1, &particleSystem.subParticle->descSet, 0, 0);
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 1, 1, &particleSystem.subParticle->material->descSet, 0, 0);
			particleSystem.DrawSubParticles(&cmd);
		}
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo->pipelines["flame"]);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 0, 1, &particleSystem.particle->descSet, 0, 0);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 1, 1, &particleSystem.particle->material->descSet, 0, 0);
		particleSystem.DrawParticles(&cmd);
	}

	//绘制GUI
	imgui->DrawFrame(cmd);

	cmd.endRenderPass();

	if (bloom) {
		bloom->ExtractionBrightness(&cmd);
		bloom->BlurH(&cmd);
		bloom->BlurV(&cmd);
	}

	renderPassBeginInfo.setClearValueCount(1);
	renderPassBeginInfo.setPClearValues(clearValue);
	renderPassBeginInfo.setFramebuffer(vkInfo->finalFramebuffers[currentBuffer]);
	renderPassBeginInfo.setRenderPass(vkInfo->finalPass);
	cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo->pipelines["final"]);

	//进行后处理
	if (bloom) {
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["final"], 0, 1, &bloom->GetSourceDescriptor(), 0, 0);
		cmd.draw(4, 1, 0, 0);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["final"], 0, 1, &vkInfo->finalPassDescSets[0], 0, 0);
		cmd.draw(4, 1, 0, 0);
	}
	else {
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["final"], 0, 1, &vkInfo->finalPassDescSets[0], 0, 0);
		cmd.draw(4, 1, 0, 0);
	}
	
	cmd.endRenderPass();
}