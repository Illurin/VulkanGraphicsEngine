#include "Scene.h"

GameObject* Scene::CreatePlane(std::string name, float width, float depth, float texRepeatX, float texRepeatY) {
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData meshData = geoGen.CreatePlane(width, depth, texRepeatX, texRepeatY);

	GameObject gameObject;
	gameObject.name = name;
	gameObject.objCBIndex = gameObjects.size();

	MeshRenderer meshRenderer;
	meshRenderer.gameObject = &gameObject;
	meshRenderer.vertices = meshData.vertices;
	meshRenderer.indices = meshData.indices;
	meshRenderers.push_back(meshRenderer);

	gameObjects[name] = gameObject;

	return &gameObject;
}

GameObject* Scene::CreateGeosphere(std::string name, float radius, uint32_t numSubdivison) {
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData meshData = geoGen.CreateGeosphere(radius, numSubdivison);

	GameObject gameObject;
	gameObject.name = name;
	gameObject.objCBIndex = gameObjects.size();

	MeshRenderer meshRenderer;
	meshRenderer.gameObject = &gameObject;
	meshRenderer.vertices = meshData.vertices;
	meshRenderer.indices = meshData.indices;
	meshRenderers.push_back(meshRenderer);

	gameObjects[name] = gameObject;

	return &gameObject;
}

Material* Scene::CreateMaterial(std::string name, Texture* diffuse, glm::mat4x4 matTransform, glm::vec4 diffuseAlbedo, glm::vec3 fresnelR0, float roughness) {
	Material material;
	material.name = name;
	material.matCBIndex = materials.size();
	material.diffuse = diffuse;
	material.diffuseAlbedo = diffuseAlbedo;
	material.fresnelR0 = fresnelR0;
	material.matTransform = matTransform;
	material.roughness = roughness;
	materials[name] = material;
	return &material;
}

void Scene::BindMaterial(GameObject* gameObject, Material* material) {
	gameObject->material = material;
}

void Scene::BindMaterial(std::string gameObject, std::string material) {
	gameObjects[gameObject].material = &materials[material];
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

void Scene::UpdateObjectConstants() {
	for (auto& gameObject : gameObjects) {
		if (gameObject.second.dirtyFlag) {
			glm::mat4x4 LR = glm::rotate(glm::mat4(1.0f), gameObject.second.transform.localEulerAngle.x, glm::vec3(1.0f, 0.0f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), gameObject.second.transform.localEulerAngle.y, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), gameObject.second.transform.localEulerAngle.z, glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4x4 S = glm::scale(glm::mat4(1.0f), glm::vec3(gameObject.second.transform.scale));
			glm::mat4x4 T = glm::translate(glm::mat4(1.0f), glm::vec3(gameObject.second.transform.position));
			glm::mat4x4 GR = glm::rotate(glm::mat4(1.0f), gameObject.second.transform.eulerAngle.x, glm::vec3(1.0f, 0.0f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), gameObject.second.transform.eulerAngle.y, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), gameObject.second.transform.eulerAngle.z, glm::vec3(0.0f, 0.0f, 1.0f));
			ObjectConstants objectConstants;
			objectConstants.worldMatrix = GR * T * S * LR;
			frameResources[0]->objCB[gameObject.second.objCBIndex]->CopyData(&vkInfo->device, 0, 1, &objectConstants);
		}
	}
}

void Scene::UpdatePassConstants() {
	PassConstants passConstants;
	passConstants.projMatrix = shadowMap.GetLightProjMatrix();
	passConstants.viewMatrix = shadowMap.GetLightViewMatrix();
	frameResources[0]->passCB[1]->CopyData(&vkInfo->device, 0, 1, &passConstants);
	passConstants.projMatrix = mainCamera->GetProjMatrix4x4();
	passConstants.viewMatrix = mainCamera->GetViewMatrix4x4();
	passConstants.eyePos = glm::vec4(mainCamera->GetPosition3f(), 1.0f);
	passConstants.shadowTransform = shadowMap.GetShadowTransform();
	passConstants.lights[0] = directionalLight;
	passConstants.lights[1] = pointLight;
	passConstants.lights[2] = spotLight;
	passConstants.ambientLight = glm::vec4(ambientLight, 1.0f);
	frameResources[0]->passCB[0]->CopyData(&vkInfo->device, 0, 1, &passConstants);
}

void Scene::UpdateMaterialConstants() {
	for (auto& material : materials) {
		if (material.second.dirtyFlag) {
			MaterialConstants materialConstants;
			materialConstants.diffuseAlbedo = material.second.diffuseAlbedo;
			materialConstants.fresnelR0 = material.second.fresnelR0;
			materialConstants.matTransform = material.second.matTransform;
			materialConstants.roughness = material.second.roughness;
			frameResources[0]->matCB[material.second.matCBIndex]->CopyData(&vkInfo->device, 0, 1, &materialConstants);
		}
	}
}

void Scene::SetupVertexBuffer() {
	std::vector<Vertex> vertices;
	std::vector<SkinnedVertex> skinnedVertices;
	std::vector<uint32_t> indices;

	for (auto& meshRenderer : meshRenderers) {
		meshRenderer.baseVertexLocation = vertices.size();
		meshRenderer.startIndexLocation = indices.size();
		vertices.insert(vertices.begin(), meshRenderer.vertices.begin(), meshRenderer.vertices.end());
		indices.insert(indices.begin(), meshRenderer.indices.begin(), meshRenderer.indices.end());
	}
	for (auto& meshRenderer : skinnedMeshRenderers) {
		meshRenderer.baseVertexLocation = skinnedVertices.size();
		meshRenderer.startIndexLocation = indices.size();
		skinnedVertices.insert(skinnedVertices.begin(), meshRenderer.vertices.begin(), meshRenderer.vertices.end());
		indices.insert(indices.begin(), meshRenderer.indices.begin(), meshRenderer.indices.end());
	}

	vk::MemoryPropertyFlags memProp = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
	vk::PhysicalDeviceMemoryProperties gpuProp = vkInfo->gpu.getMemoryProperties();

	if (vertexBuffer != nullptr)
		vertexBuffer->DestroyBuffer(&vkInfo->device);
	vertexBuffer = std::make_unique<Buffer<Vertex>>(&vkInfo->device, vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer, gpuProp, memProp, false);
	vertexBuffer->CopyData(&vkInfo->device, 0, vertices.size(), vertices.data());

	//if (skinnedVertexBuffer != nullptr)
	//	skinnedVertexBuffer->DestroyBuffer(&vkInfo->device);
	//skinnedVertexBuffer = std::make_unique<Buffer<SkinnedVertex>>(&vkInfo->device, skinnedVertices.size(), vk::BufferUsageFlagBits::eVertexBuffer, gpuProp, memProp, false);
	//skinnedVertexBuffer->CopyData(&vkInfo->device, 0, skinnedVertices.size(), skinnedVertices.data());

	if (indexBuffer != nullptr)
		indexBuffer->DestroyBuffer(&vkInfo->device);
	indexBuffer = std::make_unique<Buffer<uint32_t>>(&vkInfo->device, indices.size(), vk::BufferUsageFlagBits::eIndexBuffer, gpuProp, memProp, false);
	indexBuffer->CopyData(&vkInfo->device, 0, indices.size(), indices.data());
}

void Scene::SetupDescriptors() {
	//初始化FrameBuffer
	frameResources.resize(1);
	frameResources[0] = std::make_unique<FrameResource>(&vkInfo->device, vkInfo->gpu.getMemoryProperties(), 2, gameObjects.size(), materials.size(), skinnedModelInst.size());

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
	vkInfo->descSetLayout.resize(4);

	auto descLayoutInfo_obj = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(3)
		.setPBindings(layoutBindingObject);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_obj, 0, &vkInfo->descSetLayout[0]);

	auto descLayoutInfo_pass = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(3)
		.setPBindings(layoutBindingPass);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_pass, 0, &vkInfo->descSetLayout[1]);

	auto descLayoutInfo_shadow = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(2)
		.setPBindings(layoutBindingShadow);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_shadow, 0, &vkInfo->descSetLayout[2]);

	auto descLayoutInfo_skinned = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(1)
		.setPBindings(&layoutBindingSkinned);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_skinned, 0, &vkInfo->descSetLayout[3]);

	//为finalPass提供管线布局
	auto descLayoutInfo_final = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(2)
		.setPBindings(layoutBindingFinal);
	vkInfo->device.createDescriptorSetLayout(&descLayoutInfo_final, 0, &vkInfo->finalPassLayout);

	//为描述符的分配提供布局
	passCount = passCountPerFrame * frameCount;
	descCount = passCount + gameObjects.size();

	std::vector<vk::DescriptorSetLayout> descLayout(descCount + frameCount + skinnedModelInst.size());
	for (uint32_t i = 0; i < gameObjects.size(); i++)
		descLayout[i] = vkInfo->descSetLayout[0];
	for (uint32_t i = 0; i < passCount; i++) {
		uint32_t index = i + gameObjects.size();
		descLayout[index] = vkInfo->descSetLayout[1];
	}
	for (uint32_t i = 0; i < frameCount; i++) {
		uint32_t index = i + gameObjects.size() + passCount;
		descLayout[index] = vkInfo->descSetLayout[2];
	}
	for (uint32_t i = 0; i < skinnedModelInst.size(); i++) {
		uint32_t index = i + gameObjects.size() + passCount + frameCount;
		descLayout[index] = vkInfo->descSetLayout[3];
	}

	//创建描述符池
	vk::DescriptorPoolSize typeCount[3];
	typeCount[0].setType(vk::DescriptorType::eUniformBuffer);
	typeCount[0].setDescriptorCount(descCount + gameObjects.size() + skinnedModelInst.size());
	typeCount[1].setType(vk::DescriptorType::eSampledImage);
	typeCount[1].setDescriptorCount(gameObjects.size() + frameCount + 1);
	typeCount[2].setType(vk::DescriptorType::eSampler);
	typeCount[2].setDescriptorCount(passCount * 2 + frameCount + 1);

	auto descriptorPoolInfo = vk::DescriptorPoolCreateInfo()
		.setMaxSets(descCount + frameCount + skinnedModelInst.size() + 1)
		.setPoolSizeCount(3)
		.setPPoolSizes(typeCount);
	vkInfo->device.createDescriptorPool(&descriptorPoolInfo, 0, &vkInfo->descPool);

	//分配descCount个描述符
	vkInfo->descSets.resize(descCount + frameCount + skinnedModelInst.size());

	auto descSetAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(vkInfo->descPool)
		.setDescriptorSetCount(descCount + frameCount + skinnedModelInst.size())
		.setPSetLayouts(descLayout.data());
	vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, vkInfo->descSets.data());

	//为finalPass分配一个描述符
	vkInfo->finalPassDescSets.resize(1);

	descSetAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(vkInfo->descPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&vkInfo->finalPassLayout);
	vkInfo->device.allocateDescriptorSets(&descSetAllocInfo, vkInfo->finalPassDescSets.data());

	//更新每一个描述符
	int descSetIndex = 0;
	for (auto& gameObject : gameObjects) {
		auto descriptrorObjCBInfo = vk::DescriptorBufferInfo()
			.setBuffer(frameResources[0]->objCB[gameObject.second.objCBIndex]->GetBuffer())
			.setOffset(0)
			.setRange(sizeof(ObjectConstants));

		auto descriptorImageInfo = vk::DescriptorImageInfo()
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(gameObject.second.material->diffuse->GetImageView(&vkInfo->device));

		auto descriptorMatCBInfo = vk::DescriptorBufferInfo()
			.setBuffer(frameResources[0]->matCB[gameObject.second.material->matCBIndex]->GetBuffer())
			.setOffset(0)
			.setRange(sizeof(MaterialConstants));

		vk::WriteDescriptorSet descSetWrites[3];
		descSetWrites[0].setDescriptorCount(1);
		descSetWrites[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		descSetWrites[0].setDstArrayElement(0);
		descSetWrites[0].setDstBinding(0);
		descSetWrites[0].setDstSet(vkInfo->descSets[descSetIndex]);
		descSetWrites[0].setPBufferInfo(&descriptrorObjCBInfo);
		descSetWrites[1].setDescriptorCount(1);
		descSetWrites[1].setDescriptorType(vk::DescriptorType::eSampledImage);
		descSetWrites[1].setDstArrayElement(0);
		descSetWrites[1].setDstBinding(1);
		descSetWrites[1].setDstSet(vkInfo->descSets[descSetIndex]);
		descSetWrites[1].setPImageInfo(&descriptorImageInfo);
		descSetWrites[2].setDescriptorCount(1);
		descSetWrites[2].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		descSetWrites[2].setDstArrayElement(0);
		descSetWrites[2].setDstBinding(2);
		descSetWrites[2].setDstSet(vkInfo->descSets[descSetIndex]);
		descSetWrites[2].setPBufferInfo(&descriptorMatCBInfo);
		vkInfo->device.updateDescriptorSets(3, descSetWrites, 0, 0);
		descSetIndex++;
	}

	for (uint32_t i = 0; i < passCount; i++) {
		uint32_t index = i + gameObjects.size();

		auto descriptrorPassCBInfo = vk::DescriptorBufferInfo()
			.setBuffer(frameResources[0]->passCB[i]->GetBuffer())
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
		descSetWrites[0].setDstSet(vkInfo->descSets[index]);
		descSetWrites[0].setPBufferInfo(&descriptrorPassCBInfo);
		descSetWrites[1].setDescriptorCount(1);
		descSetWrites[1].setDescriptorType(vk::DescriptorType::eSampler);
		descSetWrites[1].setDstArrayElement(0);
		descSetWrites[1].setDstBinding(1);
		descSetWrites[1].setDstSet(vkInfo->descSets[index]);
		descSetWrites[1].setPImageInfo(&descriptorRepeatSamplerInfo);
		descSetWrites[2].setDescriptorCount(1);
		descSetWrites[2].setDescriptorType(vk::DescriptorType::eSampler);
		descSetWrites[2].setDstArrayElement(0);
		descSetWrites[2].setDstBinding(2);
		descSetWrites[2].setDstSet(vkInfo->descSets[index]);
		descSetWrites[2].setPImageInfo(&descriptorBorderSamplerInfo);
		vkInfo->device.updateDescriptorSets(3, descSetWrites, 0, 0);
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
		descSetWrites[0].setDstSet(vkInfo->descSets[descCount]);
		descSetWrites[0].setPImageInfo(&descriptorSamplerInfo);
		descSetWrites[1].setDescriptorCount(1);
		descSetWrites[1].setDescriptorType(vk::DescriptorType::eSampledImage);
		descSetWrites[1].setDstArrayElement(0);
		descSetWrites[1].setDstBinding(1);
		descSetWrites[1].setDstSet(vkInfo->descSets[descCount]);
		descSetWrites[1].setPImageInfo(&descriptorShadowMapInfo);
		vkInfo->device.updateDescriptorSets(2, descSetWrites, 0, 0);
	}
	//{
	//	uint32_t index = descCount + frameCount;
	//
	//	auto descriptrorSkinnedCBInfo = vk::DescriptorBufferInfo()
	//		.setBuffer(frameResources[0]->skinnedCB[0]->GetBuffer())
	//		.setOffset(0)
	//		.setRange(sizeof(SkinnedConstants));
	//
	//	vk::WriteDescriptorSet descSetWrites[1];
	//	descSetWrites[0].setDescriptorCount(1);
	//	descSetWrites[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
	//	descSetWrites[0].setDstArrayElement(0);
	//	descSetWrites[0].setDstBinding(0);
	//	descSetWrites[0].setDstSet(vkInfo->descSets[index]);
	//	descSetWrites[0].setPBufferInfo(&descriptrorSkinnedCBInfo);
	//	vkInfo->device.updateDescriptorSets(1, descSetWrites, 0, 0);
	//}

	//更新FinalPass的描述符
	{
		auto descriptorSamplerInfo = vk::DescriptorImageInfo()
			.setSampler(repeatSampler);

		auto descriptorImageInfo = vk::DescriptorImageInfo()
			.setImageView(vkInfo->scene.imageView)
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

	vkInfo->vertex.attrib.resize(3);

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
	//vk::VertexInputBindingDescription particleBinding;
	//std::vector<vk::VertexInputAttributeDescription> particleAttrib;

	//particleBinding.setBinding(0);
	//particleBinding.setInputRate(vk::VertexInputRate::eVertex);
	//particleBinding.setStride(sizeof(ParticleSystem::Particle));

	//particleAttrib.resize(5);

	////glm::vec3 position
	//particleAttrib[0].setBinding(0);
	//particleAttrib[0].setFormat(vk::Format::eR32G32B32Sfloat);
	//particleAttrib[0].setLocation(0);
	//particleAttrib[0].setOffset(0);

	////float size
	//particleAttrib[1].setBinding(0);
	//particleAttrib[1].setFormat(vk::Format::eR32Sfloat);
	//particleAttrib[1].setLocation(1);
	//particleAttrib[1].setOffset(sizeof(glm::vec3));

	////glm::vec4 color
	//particleAttrib[2].setBinding(0);
	//particleAttrib[2].setFormat(vk::Format::eR32G32B32A32Sfloat);
	//particleAttrib[2].setLocation(2);
	//particleAttrib[2].setOffset(sizeof(glm::vec3) + sizeof(float));

	////glm::vec4 texCoord
	//particleAttrib[3].setBinding(0);
	//particleAttrib[3].setFormat(vk::Format::eR32G32B32A32Sfloat);
	//particleAttrib[3].setLocation(3);
	//particleAttrib[3].setOffset(sizeof(glm::vec4) + sizeof(glm::vec3) + sizeof(float));

	/*Create pipelines*/
	auto vsModule = CreateShaderModule("Shaders\\vertex.spv", vkInfo->device);
	auto psModule = CreateShaderModule("Shaders\\fragment.spv", vkInfo->device);

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

	//Dynamic state
	auto dynamicInfo = vk::PipelineDynamicStateCreateInfo();
	std::vector<vk::DynamicState> dynamicStates;

	//Vertex input state
	auto viInfo = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptionCount(1)
		.setPVertexBindingDescriptions(&vkInfo->vertex.binding)
		.setVertexAttributeDescriptionCount(3)
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
	if (vkInfo->device.createPipelineLayout(&plInfo, 0, &vkInfo->pipelineLayout["scene"]) != vk::Result::eSuccess) {
		MessageBox(0, L"Create pipeline layout failed!!!", 0, 0);
	}

	//Create pipeline state
	vkInfo->pipelines["opaque"] = CreateGraphicsPipeline(vkInfo->device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo->pipelineLayout["scene"], pipelineShaderInfo, vkInfo->scenePass);
	vkInfo->device.destroyShaderModule(vsModule);
	vkInfo->device.destroyShaderModule(psModule);

	//编译用于蒙皮动画的着色器
	vsModule = CreateShaderModule("Shaders\\skinnedVS.spv", vkInfo->device);
	psModule = CreateShaderModule("Shaders\\skinnedPS.spv", vkInfo->device);

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

	vkInfo->pipelines["skinned"] = CreateGraphicsPipeline(vkInfo->device, dynamicInfo, skinnedviInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo->pipelineLayout["scene"], pipelineShaderInfo, vkInfo->scenePass);
	vkInfo->device.destroyShaderModule(vsModule);
	vkInfo->device.destroyShaderModule(psModule);

	//编译用于天空球的着色器
	vsModule = CreateShaderModule("Shaders\\skyboxVS.spv", vkInfo->device);
	psModule = CreateShaderModule("Shaders\\skyboxPS.spv", vkInfo->device);

	pipelineShaderInfo[0].setModule(vsModule);
	pipelineShaderInfo[1].setModule(psModule);

	//创建用于绘制天空球的管线
	dsInfo.setDepthCompareOp(vk::CompareOp::eLessOrEqual);

	vkInfo->pipelines["skybox"] = CreateGraphicsPipeline(vkInfo->device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo->pipelineLayout["scene"], pipelineShaderInfo, vkInfo->scenePass);

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
	/*dsInfo = vk::PipelineDepthStencilStateCreateInfo()
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

	vkInfo->pipelines["smoke"] = CreateGraphicsPipeline(vkInfo->device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo->pipelineLayout["scene"], pipelineShaderInfo, vkInfo->scenePass);

	attState.setDstColorBlendFactor(vk::BlendFactor::eOne);

	vkInfo->pipelines["flame"] = CreateGraphicsPipeline(vkInfo->device, dynamicInfo, viInfo, iaInfo, rsInfo, cbInfo, vpInfo, dsInfo, msInfo, vkInfo->pipelineLayout["scene"], pipelineShaderInfo, vkInfo->scenePass);

	vkInfo->device.destroyShaderModule(vsModule);
	vkInfo->device.destroyShaderModule(gsModule);
	vkInfo->device.destroyShaderModule(psModule);*/
}

void Scene::DrawObject(uint32_t currentBuffer) {
	shadowMap.BeginRenderPass(&vkInfo->cmd);

	vkInfo->cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 1, 1, &vkInfo->descSets[gameObjects.size() + 1], 0, 0);
	vkInfo->cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo->pipelines["shadow"]);
	
	vk::DeviceSize offsets[] = { 0 };
	vkInfo->cmd.bindIndexBuffer(indexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
	vkInfo->cmd.bindVertexBuffers(0, 1, &vertexBuffer->GetBuffer(), offsets);

	int descSetIndex = 0;
	for (auto& meshRenderer : meshRenderers) {
		vkInfo->cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 0, 1, &vkInfo->descSets[descSetIndex], 0, 0);
		vkInfo->cmd.drawIndexed(meshRenderer.indices.size(), 1, meshRenderer.startIndexLocation, meshRenderer.baseVertexLocation, 1);
		descSetIndex++;
	}

	vkInfo->cmd.endRenderPass();

	//17.6 Begin render pass
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
	vkInfo->cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

	//17.7 Bind Pipeline state
	vkInfo->cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo->pipelines["opaque"]);

	//绑定PassConstants描述符
	vkInfo->cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 1, 1, &vkInfo->descSets[gameObjects.size()], 0, 0);
	vkInfo->cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 2, 1, &vkInfo->descSets[descCount], 0, 0);

	descSetIndex = 0;
	for (auto& meshRenderer : meshRenderers) {
		vkInfo->cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["scene"], 0, 1, &vkInfo->descSets[descSetIndex], 0, 0);
		vkInfo->cmd.drawIndexed(meshRenderer.indices.size(), 1, meshRenderer.startIndexLocation, meshRenderer.baseVertexLocation, 1);
		descSetIndex++;
	}

	vkInfo->cmd.endRenderPass();

	renderPassBeginInfo.setClearValueCount(1);
	renderPassBeginInfo.setPClearValues(clearValue);
	renderPassBeginInfo.setFramebuffer(vkInfo->finalFramebuffers[currentBuffer]);
	renderPassBeginInfo.setRenderPass(vkInfo->finalPass);
	vkInfo->cmd.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

	vkInfo->cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkInfo->pipelines["final"]);
	vkInfo->cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, vkInfo->pipelineLayout["final"], 0, 1, &vkInfo->finalPassDescSets[0], 0, 0);
	vkInfo->cmd.draw(4, 1, 0, 0);

	vkInfo->cmd.endRenderPass();
}