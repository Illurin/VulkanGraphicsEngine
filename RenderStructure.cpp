#include "RenderStructure.h"

void BuildRenderItems(vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp, SkinnedModel& model, std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geo, std::vector<std::unique_ptr<Material>>& materials, std::vector<std::unique_ptr<RenderItem>>& ritems) {
	//加载模型
	std::vector<Vertex> vertexData;
	std::vector<uint32_t> indexData;

	//几何图形创建辅助类
	GeometryGenerator geoGenerator;

	//添加模型的所有材质并完成加载
	uint32_t diffuseIndex = 0;

	//添加模型的所有材质
	for (uint32_t i = 0; i < model.materials.size(); i++) {
		auto material = std::make_unique<Material>();

		material->diffuseIndex = diffuseIndex;
		material->matCBIndex = diffuseIndex;
		material->diffuseAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		material->fresnelR0 = glm::vec3(0.0f, 0.0f, 0.0f);
		material->matTransform = glm::mat4(1.0f);
		material->roughness = 0.8f;

		materials.push_back(std::move(material));
		diffuseIndex++;
	}
	//添加球体的材质
	{
		auto material = std::make_unique<Material>();

		material->diffuseIndex = diffuseIndex;
		material->matCBIndex = diffuseIndex;
		material->diffuseAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		material->fresnelR0 = glm::vec3(0.05f, 0.05f, 0.05f);
		material->matTransform = glm::mat4(1.0f);
		material->roughness = 0.8f;

		materials.push_back(std::move(material));
		diffuseIndex++;
	}
	//添加地砖的材质
	{
		auto material = std::make_unique<Material>();

		material->diffuseIndex = diffuseIndex;
		material->matCBIndex = diffuseIndex;
		material->diffuseAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		material->fresnelR0 = glm::vec3(0.5f, 0.5f, 0.5f);
		material->matTransform = glm::mat4(1.0f);
		material->roughness = 0.01f;

		materials.push_back(std::move(material));
		diffuseIndex++;
	}
	//添加粒子的材质
	{
		//flame
		auto material = std::make_unique<Material>();

		material->diffuseIndex = diffuseIndex;
		material->matCBIndex = diffuseIndex;

		materials.push_back(std::move(material));
		diffuseIndex++;
	}
	{
		//smoke
		auto material = std::make_unique<Material>();

		material->diffuseIndex = diffuseIndex;
		material->matCBIndex = diffuseIndex;

		materials.push_back(std::move(material));
		diffuseIndex++;
	}
	//添加天空球的材质
	{
		auto material = std::make_unique<Material>();

		material->diffuseIndex = diffuseIndex;
		material->matCBIndex = diffuseIndex;

		materials.push_back(std::move(material));
		diffuseIndex++;
	}

	geo["normal"] = std::make_unique<MeshGeometry>();
	geo["skinned"] = std::make_unique<MeshGeometry>();

	//创建模型的渲染项并添加模型的所有顶点
	uint32_t currentObjIndex = 0;

	//添加模型的所有渲染项
	std::vector<SkinnedVertex> skinnedVertices;
	std::vector<uint32_t> skinnedIndices;
	for (uint32_t i = 0; i < model.renderInfo.size(); i++) {
		auto ritem = std::make_unique<RenderItem>();

		ritem->layer = RenderLayer::skinned;

		ritem->geo = geo["skinned"].get();
		ritem->objCBIndex = currentObjIndex;
		ritem->baseVertexLocation = skinnedVertices.size();
		ritem->startIndexLocation = skinnedIndices.size();
		ritem->indexCount = model.renderInfo[i].indices.size();
		ritem->texTransform = glm::mat4x4(1.0f);
		ritem->worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, -1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.001f, 0.001f, 0.001f));
		ritem->material = materials[currentObjIndex].get();

		ritems.push_back(std::move(ritem));
		currentObjIndex++;

		skinnedVertices.insert(skinnedVertices.end(), model.renderInfo[i].vertices.begin(), model.renderInfo[i].vertices.end());
		skinnedIndices.insert(skinnedIndices.end(), model.renderInfo[i].indices.begin(), model.renderInfo[i].indices.end());
	}
	//球体
	{
		//创建出球体的顶点
		GeometryGenerator::MeshData meshData = geoGenerator.CreateGeosphere(0.5f, 8);

		//创建渲染项，并添加顶点和索引
		auto ritem = std::make_unique<RenderItem>();

		ritem->layer = RenderLayer::opaque;

		ritem->geo = geo["normal"].get();
		ritem->objCBIndex = currentObjIndex;
		ritem->baseVertexLocation = vertexData.size();
		ritem->startIndexLocation = indexData.size();
		ritem->indexCount = meshData.indices.size();
		ritem->texTransform = glm::mat4x4(1.0f);
		ritem->material = materials[currentObjIndex].get();

		ritems.push_back(std::move(ritem));
		currentObjIndex++;

		vertexData.insert(vertexData.end(), meshData.vertices.begin(), meshData.vertices.end());
		indexData.insert(indexData.end(), meshData.indices.begin(), meshData.indices.end());
	}
	//地板
	{
		//创建地板的顶点
		GeometryGenerator::MeshData meshData = geoGenerator.CreatePlane(30.0f, 30.0f, 10, 10);

		//创建渲染项，并添加顶点和索引
		auto ritem = std::make_unique<RenderItem>();

		ritem->layer = RenderLayer::opaque;

		ritem->geo = geo["normal"].get();
		ritem->objCBIndex = currentObjIndex;
		ritem->baseVertexLocation = vertexData.size();
		ritem->startIndexLocation = indexData.size();
		ritem->indexCount = meshData.indices.size();
		ritem->texTransform = glm::mat4x4(1.0f);
		ritem->worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		ritem->material = materials[currentObjIndex].get();

		ritems.push_back(std::move(ritem));
		currentObjIndex++;

		vertexData.insert(vertexData.end(), meshData.vertices.begin(), meshData.vertices.end());
		indexData.insert(indexData.end(), meshData.indices.begin(), meshData.indices.end());
	}
	//粒子
	{
		//flame
		auto ritem = std::make_unique<RenderItem>();

		ritem->layer = RenderLayer::particle;

		ritem->objCBIndex = currentObjIndex;
		ritem->texTransform = glm::mat4x4(1.0f);
		ritem->worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 0.0f));
		ritem->material = materials[currentObjIndex].get();

		ritems.push_back(std::move(ritem));
		currentObjIndex++;
	}
	{
		//smoke
		auto ritem = std::make_unique<RenderItem>();

		ritem->layer = RenderLayer::particle;

		ritem->objCBIndex = currentObjIndex;
		ritem->texTransform = glm::mat4x4(1.0f);
		ritem->worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 0.0f));
		ritem->material = materials[currentObjIndex].get();

		ritems.push_back(std::move(ritem));
		currentObjIndex++;
	}
	//天空球
	{
		//创建出球体的顶点
		GeometryGenerator::MeshData meshData = geoGenerator.CreateGeosphere(0.5f, 6);

		//创建渲染项，并添加顶点和索引
		auto ritem = std::make_unique<RenderItem>();

		ritem->layer = RenderLayer::skybox;

		ritem->geo = geo["normal"].get();
		ritem->objCBIndex = currentObjIndex;
		ritem->baseVertexLocation = vertexData.size();
		ritem->startIndexLocation = indexData.size();
		ritem->indexCount = meshData.indices.size();
		ritem->texTransform = glm::mat4(1.0f);
		ritem->worldMatrix = glm::mat4(1.0f);
		ritem->material = materials[currentObjIndex].get();

		ritems.push_back(std::move(ritem));
		currentObjIndex++;

		vertexData.insert(vertexData.end(), meshData.vertices.begin(), meshData.vertices.end());
		indexData.insert(indexData.end(), meshData.indices.begin(), meshData.indices.end());
	}

	//创建MeshGeometry
	vk::MemoryPropertyFlags memProp = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
	Buffer<Vertex> vertexBuffer(device, vertexData.size(), vk::BufferUsageFlagBits::eVertexBuffer, gpuProp, memProp, false);
	Buffer<uint32_t> indexBuffer(device, indexData.size(), vk::BufferUsageFlagBits::eIndexBuffer, gpuProp, memProp, false);

	vertexBuffer.CopyData(device, 0, vertexData.size(), vertexData.data());
	indexBuffer.CopyData(device, 0, indexData.size(), indexData.data());

	geo["normal"]->vertexBuffer = vertexBuffer.GetBuffer();
	geo["normal"]->indexBuffer = indexBuffer.GetBuffer();

	Buffer<SkinnedVertex> skinnedVertexBuffer(device, skinnedVertices.size(), vk::BufferUsageFlagBits::eVertexBuffer, gpuProp, memProp, false);
	Buffer<uint32_t> skinnedIndexBuffer(device, skinnedIndices.size(), vk::BufferUsageFlagBits::eIndexBuffer, gpuProp, memProp, false);

	skinnedVertexBuffer.CopyData(device, 0, skinnedVertices.size(), skinnedVertices.data());
	skinnedIndexBuffer.CopyData(device, 0, skinnedIndices.size(), skinnedIndices.data());

	geo["skinned"]->vertexBuffer = skinnedVertexBuffer.GetBuffer();
	geo["skinned"]->indexBuffer = skinnedIndexBuffer.GetBuffer();
}

void DrawRenderItems(vk::CommandBuffer* cmd, vk::PipelineLayout pipelineLayout, std::vector<vk::DescriptorSet>& descSets, std::vector<RenderItem*> ritems) {
	for (uint32_t i = 0; i < ritems.size(); i++) {
		//为每个渲染项绑定ObjectConstants
		cmd->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descSets[ritems[i]->objCBIndex], 0, 0);

		//绘制
		const vk::DeviceSize offsets[1] = { 0 };
		cmd->bindVertexBuffers(0, 1, &ritems[i]->geo->vertexBuffer, offsets);
		cmd->bindIndexBuffer(ritems[i]->geo->indexBuffer, 0, ritems[i]->geo->indexFormat);
		cmd->drawIndexed(ritems[i]->indexCount, 1, ritems[i]->startIndexLocation, ritems[i]->baseVertexLocation, 0);
	}
}