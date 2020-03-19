#pragma once
#include "vkUtil.h"

class Camera {
public:
	Camera();
	Camera(float aspect);
	~Camera(){}

	glm::vec3 GetPosition3f() { return position; }
	glm::mat4x4 GetViewMatrix4x4();
	glm::mat4x4 GetProjMatrix4x4();

	void SetLens(float fovY, float aspect, float nearZ, float farZ);
	void LookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 worldUp);
	void UpdataViewMatrix();

	void Walk(float distance);
	void Strafe(float distance);
	void Pitch(float angle);
	void RotateY(float angle);

private:
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 right = { 1.0f, 0.0f, 0.0f };
	glm::vec3 up = { 0.0f, 1.0f, 0.0f };
	glm::vec3 look = { 0.0f, 0.0f, 1.0f };

	glm::mat4x4 projMatrix;
	glm::mat4x4 viewMatrix;

	float nearZ = 0.0f;
	float farZ = 0.0f;
	float aspect = 0.0f;
	float fovY = 0.0f;
	float nearWindowHeight = 0.0f;
	float farWindowHeight = 0.0f;

	bool viewDirty = false;
};