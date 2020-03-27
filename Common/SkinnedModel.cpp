#include "SkinnedModel.h"

SkinnedModel::SkinnedModel(std::string path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	directory = path.substr(0, path.find_last_of('\\')) + '\\';
	ReadNodeHierarchy(scene->mRootNode, -1);
	ProcessNode(scene, scene->mRootNode);
	SetupRenderInfo();
	LoadAnimations(scene);
}

void SkinnedModel::ProcessNode(const aiScene* scene, aiNode* node) {
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(scene, mesh));
	}

	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(scene, node->mChildren[i]);
	}
}

SkinnedMesh SkinnedModel::ProcessMesh(const aiScene* scene, aiMesh* mesh) {
	std::vector<SkinnedVertex> vertices;
	std::vector<UINT> indices;

	std::vector<BoneData> vertexBoneData(mesh->mNumVertices);
	LoadBones(mesh, vertexBoneData);

	for (size_t i = 0; i < mesh->mNumVertices; i++) {
		SkinnedVertex vertex;
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;

		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;

		vertex.tangent.x = mesh->mTangents[i].x;
		vertex.tangent.y = mesh->mTangents[i].y;
		vertex.tangent.z = mesh->mTangents[i].z;

		if (mesh->mTextureCoords[0]) {
			vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
			vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
		}
		else {
			vertex.texCoord = { 0.0f, 0.0f };
		}
		vertex.boneWeights.x = vertexBoneData[i].weights[0];
		vertex.boneWeights.y = vertexBoneData[i].weights[1];
		vertex.boneWeights.z = vertexBoneData[i].weights[2];

		for (size_t j = 0; j < NUM_BONES_PER_VERTEX; j++)
			vertex.boneIndices[j] = vertexBoneData[i].boneIndex[j];

		vertices.push_back(vertex);
	}

	for (size_t i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	std::vector<UINT> diffuseMaps;
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE);
	}
	UINT materialIndex = SetupMaterial(diffuseMaps);

	return SkinnedMesh(vertices, indices, materialIndex);
}

void SkinnedModel::LoadBones(const aiMesh* mesh, std::vector<SkinnedModel::BoneData>& boneData) {
	for (size_t i = 0; i < mesh->mNumBones; i++) {
		UINT boneIndex = 0;
		std::string boneName(mesh->mBones[i]->mName.C_Str());

		if (boneMapping.find(boneName) == boneMapping.end()) {
			//insert error program
			MessageBox(NULL, L"cannot find node", NULL, NULL);
		}
		else
			boneIndex = boneMapping[boneName];

		boneMapping[boneName] = boneIndex;

		if (!boneInfo[boneIndex].isSkinned) {
			aiMatrix4x4 offsetMatrix = mesh->mBones[i]->mOffsetMatrix;
			boneInfo[boneIndex].boneOffset = {
				offsetMatrix.a1, offsetMatrix.b1, offsetMatrix.c1, offsetMatrix.d1,
				offsetMatrix.a2, offsetMatrix.b2, offsetMatrix.c2, offsetMatrix.d2,
				offsetMatrix.a3, offsetMatrix.b3, offsetMatrix.c3, offsetMatrix.d3,
				offsetMatrix.a4, offsetMatrix.b4, offsetMatrix.c4, offsetMatrix.d4
			};
			boneInfo[boneIndex].isSkinned = true;
		}

		for (size_t j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
			UINT vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
			float weight = mesh->mBones[i]->mWeights[j].mWeight;
			boneData[vertexID].Add(boneIndex, weight);
		}
	}
}

void SkinnedModel::ReadNodeHierarchy(const aiNode* node, int parentIndex) {
	BoneInfo boneInfo;

	UINT boneIndex = this->boneInfo.size();
	boneMapping[node->mName.C_Str()] = boneIndex;

	boneInfo.boneOffset = glm::mat4(1.0f);
	boneInfo.parentIndex = parentIndex;
	boneInfo.defaultOffset = {
		node->mTransformation.a1, node->mTransformation.b1, node->mTransformation.c1, node->mTransformation.d1,
		node->mTransformation.a2, node->mTransformation.b2, node->mTransformation.c2, node->mTransformation.d2,
		node->mTransformation.a3, node->mTransformation.b3, node->mTransformation.c3, node->mTransformation.d3,
		node->mTransformation.a4, node->mTransformation.b4, node->mTransformation.c4, node->mTransformation.d4
	};
	this->boneInfo.push_back(boneInfo);

	for (size_t i = 0; i < node->mNumChildren; i++)
		ReadNodeHierarchy(node->mChildren[i], boneIndex);
}

std::vector<UINT> SkinnedModel::LoadMaterialTextures(aiMaterial* mat, aiTextureType type) {
	std::vector<UINT> textures;
	for (UINT i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, 0, &str);
		bool skip = false;
		std::string texturePath = directory + str.C_Str();
		for (UINT j = 0; j < this->texturePath.size(); j++)
		{
			if (this->texturePath[j] == texturePath) {
				textures.push_back(j);
				skip = true;
				break;
			}
		}
		if (!skip) {
			textures.push_back(this->texturePath.size());
			this->texturePath.push_back(texturePath);
		}
	}
	return textures;
}

UINT SkinnedModel::SetupMaterial(std::vector<UINT> diffuseMaps) {
	MaterialInfo material;
	if (diffuseMaps.size() > 0)
		material.diffuseMaps = diffuseMaps[0];
	else
		material.diffuseMaps = 0;

	UINT materialIndex;
	bool skip = false;
	for (UINT i = 0; i < materials.size(); i++) {
		if (CompareMaterial(materials[i], material)) {
			materialIndex = i;
			skip = true;
			break;
		}
	}
	if (!skip) {
		materialIndex = materials.size();
		materials.push_back(material);
	}
	return materialIndex;
}

void SkinnedModel::SetupRenderInfo() {
	renderInfo.resize(materials.size());

	for (UINT i = 0; i < meshes.size(); i++)
	{
		UINT index = meshes[i].materialIndex;
		UINT indexOffset = renderInfo[index].vertices.size();

		for (UINT j = 0; j < meshes[i].indices.size(); j++)
			renderInfo[index].indices.push_back(meshes[i].indices[j] + indexOffset);

		renderInfo[index].vertices.insert(renderInfo[index].vertices.end(), meshes[i].vertices.begin(), meshes[i].vertices.end());
	}
}

void SkinnedModel::LoadAnimations(const aiScene* scene) {
	for (size_t i = 0; i < scene->mNumAnimations; i++) {
		AnimationClip animation;
		std::vector<BoneAnimation> boneAnims(boneInfo.size());
		aiAnimation* anim = scene->mAnimations[i];

		float ticksPerSecond = anim->mTicksPerSecond != 0 ? anim->mTicksPerSecond : 25.0f;
		float timeInTicks = 1.0f / ticksPerSecond;

		for (size_t j = 0; j < anim->mNumChannels; j++) {
			BoneAnimation boneAnim;
			aiNodeAnim* nodeAnim = anim->mChannels[j];

			for (size_t k = 0; k < nodeAnim->mNumPositionKeys; k++) {
				VectorKey keyframe;
				keyframe.timePos = nodeAnim->mPositionKeys[k].mTime * timeInTicks;
				keyframe.value.x = nodeAnim->mPositionKeys[k].mValue.x;
				keyframe.value.y = nodeAnim->mPositionKeys[k].mValue.y;
				keyframe.value.z = nodeAnim->mPositionKeys[k].mValue.z;
				boneAnim.translation.push_back(keyframe);
			}
			for (size_t k = 0; k < nodeAnim->mNumScalingKeys; k++) {
				VectorKey keyframe;
				keyframe.timePos = nodeAnim->mScalingKeys[k].mTime * timeInTicks;
				keyframe.value.x = nodeAnim->mScalingKeys[k].mValue.x;
				keyframe.value.y = nodeAnim->mScalingKeys[k].mValue.y;
				keyframe.value.z = nodeAnim->mScalingKeys[k].mValue.z;
				boneAnim.scale.push_back(keyframe);
			}
			for (size_t k = 0; k < nodeAnim->mNumRotationKeys; k++) {
				QuatKey keyframe;
				keyframe.timePos = nodeAnim->mRotationKeys[k].mTime * timeInTicks;
				keyframe.value.x = nodeAnim->mRotationKeys[k].mValue.x;
				keyframe.value.y = nodeAnim->mRotationKeys[k].mValue.y;
				keyframe.value.z = nodeAnim->mRotationKeys[k].mValue.z;
				keyframe.value.w = nodeAnim->mRotationKeys[k].mValue.w;
				boneAnim.rotationQuat.push_back(keyframe);
			}
			boneAnims[boneMapping[nodeAnim->mNodeName.C_Str()]] = boneAnim;
		}
		animation.boneAnimations = boneAnims;

		std::string animName(anim->mName.C_Str());
		animName = animName.substr(animName.find_last_of('|') + 1, animName.length() - 1);

		animations[animName] = animation;
	}
}