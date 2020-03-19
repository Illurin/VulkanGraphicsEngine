#include "Camera.h"

Camera::Camera() {
	SetLens(0.25f * glm::pi<float>(), 2.0f, 0.1f, 1000.0f);
}

Camera::Camera(float aspect) {
	SetLens(0.25f * glm::pi<float>(), aspect, 0.1f, 1000.0f);
}

glm::mat4x4 Camera::GetViewMatrix4x4() {
	return viewMatrix;
}

glm::mat4x4 Camera::GetProjMatrix4x4() {
	return projMatrix;
}

void Camera::SetLens(float fovY, float aspect, float nearZ, float farZ) {
	this->fovY = fovY;
	this->aspect = aspect;
	this->nearZ = nearZ;
	this->farZ = farZ;

	nearWindowHeight = 2.0f * nearZ * tanf(0.5f * fovY);
	farWindowHeight = 2.0f * farZ * tanf(0.5f * fovY);

	projMatrix = glm::perspective(fovY, aspect, nearZ, farZ);
	projMatrix[1][1] *= -1.0f;
}

void Camera::LookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 worldUp) {
	look = glm::normalize(target - pos);
	right = glm::normalize(glm::cross(worldUp, look));
	up = glm::cross(look, right);

	viewDirty = true;
}

void Camera::UpdataViewMatrix() {
	if (viewDirty) {
		look = glm::normalize(look);
		up = glm::normalize(glm::cross(look, right));
		right = glm::cross(up, look);

		viewMatrix = {
			right.x, up.x, -look.x, 0.0f,
			right.y, up.y, -look.y, 0.0f,
			right.z, up.z, -look.z, 0.0f,
			-glm::dot(position, right), -glm::dot(position, up), -glm::dot(position, -look), 1.0f
		};

		viewDirty = false;
	}
}

void Camera::Walk(float distance) {
	position = position + look * distance;
	viewDirty = true;
}

void Camera::Strafe(float distance) {
	position = position + right * distance;
	viewDirty = true;
}

void Camera::Pitch(float angle) {
	glm::mat4x4 R = glm::rotate(glm::mat4(1.0f), angle, right);
	up = glm::vec3(R * glm::vec4(up, 0.0f));
	look = glm::vec3(R * glm::vec4(look, 0.0f));

	viewDirty = true;
}

void Camera::RotateY(float angle) {
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4x4 R = glm::rotate(glm::mat4(1.0f), angle, worldUp);
	up = glm::vec3(R * glm::vec4(up, 0.0f));
	right = glm::vec3(R * glm::vec4(right, 0.0f));
	look = glm::vec3(R * glm::vec4(look, 0.0f));
	
	viewDirty = true;
}