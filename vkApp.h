#pragma once
#include "Common/Scene.h"

class VkApp {
public:
	void Initialize(uint32_t windowWidth, uint32_t windowHeight, HWND hWnd, HINSTANCE hInstance);
	void Start();
	void Loop();

private:
	void Update();
	void OnGUI();
	void PrepareFinalPass();

	Vulkan vkInfo;
	Scene scene;

	bool recordCommand = true;

	//Global variable
	Camera mainCamera;
	float deltaTime = 0.01f;

	bool particleSystemEnabled = true;
};