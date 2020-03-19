#include "SkinnedData.h"

float BoneAnimation::GetStartTime()const {
	float t0 = 0.0f;
	float t1 = 0.0f;
	float t2 = 0.0f;
	if (translation.size() != 0) t0 = translation.front().timePos;
	if (scale.size() != 0) t1 = scale.front().timePos;
	if (rotationQuat.size() != 0) t2 = rotationQuat.front().timePos;

	float timePos = t0 < t1 ? t0 : t1;
	timePos = timePos < t2 ? timePos : t2;
	return timePos;
}

float BoneAnimation::GetEndTime()const {
	float t0 = 0.0f;
	float t1 = 0.0f;
	float t2 = 0.0f;
	if (translation.size() != 0) t0 = translation.back().timePos;
	if (scale.size() != 0) t1 = scale.back().timePos;
	if (rotationQuat.size() != 0) t2 = rotationQuat.back().timePos;

	float timePos = t0 > t1 ? t0 : t1;
	timePos = timePos > t2 ? timePos : t2;
	return timePos;
}

glm::vec3 BoneAnimation::LerpKeys(float t, const std::vector<VectorKey>& keys) {
	glm::vec3 res = glm::vec3(0.0f);
	if (t <= keys.front().timePos) res = keys.front().value;
	else if (t >= keys.back().timePos) res = keys.back().value;
	else {
		for (size_t i = 0; i < keys.size() - 1; i++){
			if (t >= keys[i].timePos && t <= keys[i + 1].timePos){
				float lerpPercent = (t - keys[i].timePos) / (keys[i + 1].timePos - keys[i].timePos);
				res = keys[i].value * lerpPercent + keys[i + 1].value * (1 - lerpPercent);
				break;
			}
		}
	}
	return res;
}

glm::qua<float> BoneAnimation::LerpKeys(float t, const std::vector<QuatKey>& keys) {
	glm::qua<float> res = glm::qua<float>();
	if (t <= keys.front().timePos) res = keys.front().value;
	else if (t >= keys.back().timePos) res = keys.back().value;
	else {
		for (size_t i = 0; i < keys.size() - 1; i++) {
			if (t >= keys[i].timePos && t <= keys[i + 1].timePos) {
				float lerpPercent = (t - keys[i].timePos) / (keys[i + 1].timePos - keys[i].timePos);
				res = glm::slerp(keys[i].value, keys[i + 1].value, lerpPercent);
				break;
			}
		}
	}
	return res;
}

void BoneAnimation::Interpolate(float t, glm::mat4x4& M) {
	if (translation.size() == 0 && scale.size() == 0 && rotationQuat.size() == 0) {
		M = defaultTransform;
		return;
	}

	glm::vec3 T = glm::vec3(0.0f);
	glm::vec3 S = glm::vec3(0.0f);
	glm::qua<float> R = glm::qua<float>();

	if (translation.size() != 0) T = LerpKeys(t, translation);
	if (scale.size() != 0) S = LerpKeys(t, scale);
	if (rotationQuat.size() != 0) R = LerpKeys(t, rotationQuat);

	M = glm::translate(glm::mat4(1.0f), T) * glm::mat4_cast(R) * glm::scale(glm::mat4(1.0f), S);
}

float AnimationClip::GetClipStartTime()const {
	float t = FLT_MAX;
	for (UINT i = 0; i < boneAnimations.size(); i++)
		t = t < boneAnimations[i].GetStartTime() ? t : boneAnimations[i].GetStartTime();
	return t;
}

float AnimationClip::GetClipEndTime()const {
	float t = 0.0f;
	for (UINT i = 0; i < boneAnimations.size(); i++)
		t = t > boneAnimations[i].GetEndTime() ? t : boneAnimations[i].GetEndTime();
	return t;
}

void AnimationClip::Interpolate(float t, std::vector<glm::mat4x4>& boneTransform) {
	for (UINT i = 0; i < boneAnimations.size(); i++)
		boneAnimations[i].Interpolate(t, boneTransform[i]);
}

float SkinnedData::GetClipStartTime(const std::string& clipName)const {
	auto clip = animations.find(clipName);
	return clip->second.GetClipStartTime();
}

float SkinnedData::GetClipEndTime(const std::string& clipName)const {
	auto clip = animations.find(clipName);
	return clip->second.GetClipEndTime();
}

void SkinnedData::Set(std::vector<int>& boneHierarchy,
	std::vector<glm::mat4x4>& boneOffsets,
	std::unordered_map<std::string, AnimationClip>& animations) {
	this->boneHierarchy = boneHierarchy;
	this->boneOffsets = boneOffsets;
	this->animations = animations;
}

void SkinnedData::GetFinalTransform(const std::string& clipName, float timePos, std::vector<glm::mat4x4>& finalTransforms) {
	UINT numBones = boneOffsets.size();
	std::vector<glm::mat4x4> toParentTransforms(numBones);

	auto clip = animations.find(clipName);
	clip->second.Interpolate(timePos, toParentTransforms);

	std::vector<glm::mat4x4> toRootTransforms(numBones);
	toRootTransforms[0] = toParentTransforms[0];

	for (UINT i = 1; i < numBones; i++){
		int parentIndex = boneHierarchy[i];
		toRootTransforms[i] = toRootTransforms[parentIndex] * toParentTransforms[i];
	}

	for (UINT i = 0; i < numBones; i++) {
		finalTransforms[i] = toRootTransforms[i] * boneOffsets[i];
	}
}