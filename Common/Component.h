#pragma once
#include "vkUtil.h"
#include "Texture.h"

enum class SamplerType : int {
	repeat = 0,
	border
};

struct Transform {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	glm::vec3 eulerAngle = glm::vec3(0.0f);
	glm::vec3 localEulerAngle = glm::vec3(0.0f);
};

struct Material {
	std::string name;

	SamplerType samplerType = SamplerType::repeat;
	uint32_t matCBIndex = 0;
	Texture* diffuse;

	glm::mat4x4 matTransform = glm::mat4(1.0f);
	glm::vec4 diffuseAlbedo = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 fresnelR0 = glm::vec3(0.0f, 0.0f, 0.0f);
	float roughness = 0.0f;

	vk::DescriptorSet descSet;

	bool dirtyFlag = true;
};

struct GameObject {
	std::string name;
	Transform transform;
	Material* material;
	uint32_t objCBIndex = 0;

	GameObject* parent;
	std::vector<GameObject*> children;

	glm::mat4x4 toParent;
	vk::DescriptorSet descSet;

	bool dirtyFlag = true;
};

struct MeshRenderer {
	GameObject* gameObject;

	int baseVertexLocation;
	int startIndexLocation;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

struct SkinnedMeshRenderer {
	GameObject* gameObject;

	int baseVertexLocation;
	int startIndexLocation;

	std::vector<SkinnedVertex> vertices;
	std::vector<uint32_t> indices;
};
