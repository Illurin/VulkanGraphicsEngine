#pragma once
#include "Common/vkUtil.h"
#include "Common/SkinnedModel.h"
#include "Common/GeometryGenerator.h"
#include "Common/ParticleSystem.h"

/*===========================================‰÷»æœÓΩ·ππ===========================================*/
enum class RenderLayer : int {
	opaque = 0,
	skinned,
	skybox,
	particle,
	layerCount
};

struct MeshGeometry {
	vk::Buffer vertexBuffer = nullptr;
	vk::Buffer indexBuffer = nullptr;

	vk::IndexType indexFormat = vk::IndexType::eUint32;
};

struct RenderItem {
	RenderLayer layer;

	glm::mat4x4 worldMatrix;
	glm::mat4x4 texTransform;

	int objCBIndex;

	Material* material = nullptr;
	MeshGeometry* geo = nullptr;

	uint32_t indexCount = 0;
	uint32_t startIndexLocation = 0;
	uint32_t baseVertexLocation = 0;
};

/*===============================================================================================*/

void BuildRenderItems(vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp, SkinnedModel& model, std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geo, std::vector<std::unique_ptr<Material>>& materials, std::vector<std::unique_ptr<RenderItem>>& ritems);
void DrawRenderItems(vk::CommandBuffer* cmd, vk::PipelineLayout pipelineLayout, std::vector<vk::DescriptorSet>& descSets, std::vector<RenderItem*> ritems);