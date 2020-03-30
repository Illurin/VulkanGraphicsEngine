#pragma once
#include "vkUtil.h"
#include "Texture.h"

enum class SamplerType : int {
	repeat = 0,
	border
};

enum class ShaderModel : int {
	common = 0,
	normalMap,
	shaderModelCount
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
	ShaderModel shaderModel = ShaderModel::common;
	
	uint32_t matCBIndex = 0;
	Texture* diffuse;
	Texture* normal;

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

	GameObject* parent = nullptr;
	std::vector<GameObject*> children;

	ObjectConstants objectConstants;
	vk::DescriptorSet descSet;

	bool dirtyFlag = true;

	void UpdateData() {
		glm::mat4x4 LR = glm::rotate(glm::mat4(1.0f), transform.localEulerAngle.x, glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), transform.localEulerAngle.y, glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), transform.localEulerAngle.z, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4x4 S = glm::scale(glm::mat4(1.0f), glm::vec3(transform.scale));
		glm::mat4x4 T = glm::translate(glm::mat4(1.0f), glm::vec3(transform.position));
		glm::mat4x4 GR = glm::rotate(glm::mat4(1.0f), transform.eulerAngle.x, glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), transform.eulerAngle.y, glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), transform.eulerAngle.z, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4x4 toParent = GR * T * S * LR;
		
		objectConstants.worldMatrix = (parent ? parent->objectConstants.worldMatrix : glm::mat4(1.0f)) * toParent;
		objectConstants.worldMatrix_trans_inv = glm::transpose(glm::inverse(objectConstants.worldMatrix));

		dirtyFlag = true;

		for (auto& child : children) {
			child->UpdateData();
		}
	}
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
	uint32_t skinnedModelIndex;

	int baseVertexLocation;
	int startIndexLocation;

	std::vector<SkinnedVertex> vertices;
	std::vector<uint32_t> indices;
};
