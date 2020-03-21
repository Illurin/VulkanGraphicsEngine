#pragma once
#include "vkUtil.h"
#include "Texture.h"

struct Transform {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	glm::vec3 eulerAngle = glm::vec3(0.0f);
	glm::vec3 localEulerAngle = glm::vec3(0.0f);
};

struct Material {
	std::string name;

	uint32_t matCBIndex = 0;
	Texture* diffuse;

	glm::mat4x4 matTransform;
	glm::vec4 diffuseAlbedo;
	glm::vec3 fresnelR0;
	float roughness;

	vk::DescriptorSet descSet;

	bool dirtyFlag = true;
};

struct GameObject {
	std::string name;
	Transform transform;
	Material* material;
	uint32_t objCBIndex = 0;

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
