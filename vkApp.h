#pragma once
#include "Common/Scene.h"

class VkApp {
public:
	void Initialize(uint32_t windowWidth, uint32_t windowHeight, HWND hWnd, HINSTANCE hInstance);
	void Start();
	void Loop();

private:
	void Update();
	void PrepareScenePass();
	void PrepareFinalPass();

	Vulkan vkInfo;
	Scene scene;

	//Global value
	Camera mainCamera;

	float deltaTime = 0.01f;
};