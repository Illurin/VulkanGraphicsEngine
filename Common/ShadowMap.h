#pragma once
#include "vkUtil.h"

class ShadowMap {
public:
	ShadowMap(){}
	ShadowMap(vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp, uint32_t width, uint32_t height) {
		Init(device, gpuProp, width, height);
		PrepareRenderPass(device);
		PrepareFramebuffer(device);
	}
	void Init(vk::Device* device, vk::PhysicalDeviceMemoryProperties gpuProp, uint32_t width, uint32_t height);

	void SetLightTransformMatrix(glm::vec3 lightDirection, float radius);
	void PrepareRenderPass(vk::Device* device);
	void PrepareFramebuffer(vk::Device* device);

	void BeginRenderPass(vk::CommandBuffer* cmd);
	vk::RenderPass GetRenderPass()const { return renderPass; }
	vk::ImageView GetImageView()const { return shadowMapView; }
	glm::mat4x4 GetLightViewMatrix()const { return lightView; }
	glm::mat4x4 GetLightProjMatrix()const { return lightProj; }
	glm::mat4x4 GetShadowTransform()const { return shadowTransform; }

	void Destroy(vk::Device device) {
		device.destroy(shadowMap);
		device.destroy(shadowMapView);
		device.destroy(renderPass);
		device.destroy(framebuffer);
		device.free(memory);
	}

private:
	vk::Image shadowMap;
	vk::ImageView shadowMapView;
	vk::DeviceMemory memory;

	uint32_t width, height;
	vk::Format format = vk::Format::eD16Unorm;

	vk::RenderPass renderPass;
	vk::Framebuffer framebuffer;

	glm::mat4x4 lightView;
	glm::mat4x4 lightProj;
	glm::mat4x4 shadowTransform;
};