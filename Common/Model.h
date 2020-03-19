#pragma once

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"
#include "assimp/texture.h"
#include "Texture.h"

struct MaterialInfo {
	UINT diffuseMaps;
};

class Mesh {
public:
	struct RenderInfo {
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
	};

	std::vector<Vertex> vertices;
	std::vector<UINT> indices;
	UINT materialIndex;
	
	Mesh(std::vector<Vertex> vertices, std::vector<UINT> indices, UINT materialIndex) {
		this->vertices = vertices;
		this->indices = indices;
		this->materialIndex = materialIndex;
	}
};

class Model {
public:
	Model(std::string path);
	std::vector<Mesh> meshes;
	std::vector<MaterialInfo> materials;
	std::vector<Mesh::RenderInfo> renderInfo;
	std::vector<std::string> texturePath;

private:
	std::string directory;

	void ProcessNode(const aiScene* scene, aiNode* node);
	Mesh ProcessMesh(const aiScene* scene, aiMesh* mesh);
	UINT SetupMaterial(std::vector<UINT> diffuseMaps);
	void SetupRenderInfo();
	std::vector<UINT> LoadMaterialTextures(aiMaterial* mat, aiTextureType type);
};

bool CompareMaterial(MaterialInfo dest, MaterialInfo source);