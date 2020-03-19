#pragma once

#include "Model.h"
#include "SkinnedData.h"

class SkinnedMesh {
public:
	struct RenderInfo {
		std::vector<SkinnedVertex> vertices;
		std::vector<UINT> indices;
	};

	std::vector<SkinnedVertex> vertices;
	std::vector<UINT> indices;
	UINT materialIndex;

	SkinnedMesh(std::vector<SkinnedVertex> vertices, std::vector<UINT> indices, UINT materialIndex) {
		this->vertices = vertices;
		this->indices = indices;
		this->materialIndex = materialIndex;
	}
};

class SkinnedModel {
public:
	struct BoneData {
		UINT boneIndex[NUM_BONES_PER_VERTEX];
		float weights[NUM_BONES_PER_VERTEX];
		void Add(UINT boneID, float weight) {
			for (size_t i = 0; i < NUM_BONES_PER_VERTEX; i++) {
				if (weights[i] == 0.0f) {
					boneIndex[i] = boneID;
					weights[i] = weight;
					return;
				}
			}
			//insert error program
			MessageBox(NULL, L"bone index out of size", L"Error", NULL);
		}
	};
	struct BoneInfo {
		bool isSkinned = false;
		glm::mat4x4 boneOffset;
		glm::mat4x4 defaultOffset;
		int parentIndex;
	};

	SkinnedModel(std::string path);
	std::vector<SkinnedMesh> meshes;
	std::vector<MaterialInfo> materials;
	std::vector<SkinnedMesh::RenderInfo> renderInfo;
	std::vector<std::string> texturePath;

	void GetBoneMapping(std::unordered_map<std::string, UINT>& boneMapping) {
		boneMapping = this->boneMapping;
	}
	void GetBoneOffsets(std::vector<glm::mat4x4>& boneOffsets) {
		for (size_t i = 0; i < boneInfo.size(); i++)
			boneOffsets.push_back(boneInfo[i].boneOffset);
	}
	void GetNodeOffsets(std::vector<glm::mat4x4>& nodeOffsets) {
		for (size_t i = 0; i < boneInfo.size(); i++)
			nodeOffsets.push_back(boneInfo[i].defaultOffset);
	}
	void GetBoneHierarchy(std::vector<int>& boneHierarchy) {
		for (size_t i = 0; i < boneInfo.size(); i++)
			boneHierarchy.push_back(boneInfo[i].parentIndex);
	}
	void GetAnimations(std::unordered_map<std::string, AnimationClip>& animations) {
		animations.insert(this->animations.begin(), this->animations.end());
	}

private:
	std::string directory;

	void ProcessNode(const aiScene* scene, aiNode* node);
	SkinnedMesh ProcessMesh(const aiScene* scene, aiMesh* mesh);
	UINT SetupMaterial(std::vector<UINT> diffuseMaps);
	void SetupRenderInfo();
	std::vector<UINT> LoadMaterialTextures(aiMaterial* mat, aiTextureType type);

	//Bone/Animation Information
	std::vector<BoneInfo> boneInfo;
	std::unordered_map<std::string, UINT> boneMapping;
	std::unordered_map<std::string, AnimationClip> animations;

	void LoadBones(const aiMesh* mesh, std::vector<BoneData>& bones);
	void ReadNodeHierarchy(const aiNode* node, int parentIndex);
	void LoadAnimations(const aiScene* scene);
};