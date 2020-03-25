#pragma once
#include "vkUtil.h"

struct VectorKey {
	float timePos;
	glm::vec3 value;
};

struct QuatKey {
	float timePos;
	glm::qua<float> value;
};

class BoneAnimation {
public:
	float GetStartTime()const;
	float GetEndTime()const;
	void Interpolate(float t, glm::mat4x4& M);
	
	std::vector<VectorKey> translation;
	std::vector<VectorKey> scale;
	std::vector<QuatKey> rotationQuat;

	glm::mat4x4 defaultTransform;

private:
	glm::vec3 LerpKeys(float t, const std::vector<VectorKey>& keys);
	glm::qua<float> LerpKeys(float t, const std::vector<QuatKey>& keys);
};

class AnimationClip {
public:
	float GetClipStartTime()const;
	float GetClipEndTime()const;
	void Interpolate(float t, std::vector<glm::mat4x4>& boneTransform);
	
	std::vector<BoneAnimation> boneAnimations;
};

class SkinnedData {
public:
	UINT GetBoneCount()const { return boneHierarchy.size(); }
	float GetClipStartTime(const std::string& clipName)const;
	float GetClipEndTime(const std::string& clipName)const;
	void Set(std::vector<int>& boneHierarchy,
		std::vector<glm::mat4x4>& boneOffsets,
		std::unordered_map<std::string, AnimationClip>& animations);
	void GetFinalTransform(const std::string& clipName, float timePos, std::vector<glm::mat4x4>& finalTransforms);

private:
	std::vector<int> boneHierarchy;
	std::vector<glm::mat4x4> boneOffsets;
	std::unordered_map<std::string, AnimationClip> animations;
};

struct SkinnedModelInstance {
	SkinnedData skinnedInfo;
	std::vector<glm::mat4x4> finalTransforms;
	std::string clipName;
	float timePos = 0.0f;

	uint32_t skinnedCBIndex;
	vk::DescriptorSet descSet;

	void UpdateSkinnedAnimation(float deltaTime) {
		timePos += deltaTime;

		//Loop
		if (timePos > skinnedInfo.GetClipEndTime(clipName))
			timePos = 0.0f;

		skinnedInfo.GetFinalTransform(clipName, timePos, finalTransforms);
	}
};