#pragma once
#include "GeometryGenerator.h"
#include "SkinnedModel.h"
#include "Component.h"
#include "SkinnedData.h"
#include "FrameResource.h"
#include "ShadowMap.h"
#include "Camera.h"
#include "ParticleSystem.h"
#include "PostProcessing.h"

class Scene {
public:
	void AddGameObject(GameObject& gameObject, GameObject* parent);
	void AddMaterial(Material& material);

	void AddMeshRenderer(GameObject* gameObject, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	void AddParticleSystem(GameObject* particle, GameObject* subParticle, ParticleSystem::Property& property, ParticleSystem::Emitter& emitter, ParticleSystem::Texture& texture, ParticleSystem::SubParticle& subParticleProperty);

	//Get方法
	GameObject* GetGameObject(std::string name);
	Material* GetMaterial(std::string name);

	//光照阴影方法
	void SetAmbientLight(glm::vec3 strength);
	void SetDirectionalLight(glm::vec3 direction, glm::vec3 strength);
	void SetPointLight(glm::vec3 position, glm::vec3 strength, float fallOffStart, float fallOffEnd);
	void SetSpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 strength, float fallOffStart, float fallOffEnd, float spotPower);
	void SetShadowMap(uint32_t width, uint32_t height, glm::vec3 lightDirection, float radius);

	void SetMainCamera(Camera* mainCamera);

	//天空盒设定
	void SetSkybox(Texture image, float radius, uint32_t subdivision);

	//后处理设定
	void SetBloomPostProcessing(PostProcessingProfile::Bloom& profile);

	void UpdateObjectConstants();
	void UpdatePassConstants();
	void UpdateMaterialConstants();
	void UpdateCPUParticleSystem(float deltaTime);

	void SetupVertexBuffer();
	void SetupDescriptors();
	void PreparePipeline();

	void DrawObject(uint32_t currentBuffer);

	Vulkan* vkInfo;

private:
	void CalcCildTransform(GameObject* gameObject, glm::mat4x4 parentToWorld);

	uint32_t passCount = 2;

	std::vector<GameObject*> rootObjects;

	std::unordered_map<std::string, GameObject> gameObjects;
	std::unordered_map<std::string, Material> materials;

	std::unique_ptr<Buffer<Vertex>> vertexBuffer;
	std::unique_ptr<Buffer<SkinnedVertex>> skinnedVertexBuffer;
	std::unique_ptr<Buffer<uint32_t>> indexBuffer;

	std::unique_ptr<FrameResource> frameResources;

	//Pass描述符
	vk::DescriptorSet scenePassDesc;
	vk::DescriptorSet shadowPassDesc;
	vk::DescriptorSet drawShadowDesc;

	//组件池
	std::vector<MeshRenderer> meshRenderers;
	std::vector<SkinnedMeshRenderer> skinnedMeshRenderers;
	std::vector<SkinnedModelInstance> skinnedModelInst;
	std::vector<ParticleSystem> particleSystems;

	//场景属性
	//灯光
	glm::vec3 ambientLight;
	Light directionalLight;
	Light pointLight;
	Light spotLight;
	
	//阴影
	ShadowMap shadowMap;

	//相机
	Camera* mainCamera;

	//天空盒
	struct {
		bool use = false;
		Texture image;
		float radius;
		uint32_t subdivision;

		int startIndexLocation;
		int baseVertexLocation;
		int indexCount;

		vk::DescriptorSet descSet;
	}skybox;

	//后处理
	std::unique_ptr<PostProcessing::Bloom> bloom;
};
