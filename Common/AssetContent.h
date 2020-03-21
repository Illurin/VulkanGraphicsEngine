#pragma once
#include "vkUtil.h"
#include "Component.h"

struct Prefab {
	std::string name;
	Transform transform;

};

class AssetContent {
public:
	void LoadTexture();
	void LoadCubeMap();

private:
	Vulkan& vkInfo;

	std::unordered_map<std::string, Prefab> prefabs;
	std::unordered_map<std::string, Material> materials;
	std::unordered_map<std::string, Texture> textures;

	std::vector<MeshRenderer> meshRenderers;
	std::vector<SkinnedMeshRenderer> skinnedMeshRenderer;
};