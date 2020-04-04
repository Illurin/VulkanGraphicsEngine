#pragma once
#include "Common/Editor.h"

class VkApp {
public:
	void Initialize(uint32_t windowWidth, uint32_t windowHeight, HWND hWnd, HINSTANCE hInstance);
	void Start();
	void Loop();

private:
	void Update();
	void OnGUI();

	Vulkan vkInfo;
	Scene scene;
	Editor* engineEditor;

	bool recordCommand = true;

	//Global variable
	Camera mainCamera;
	float deltaTime = 0.05f;

	float hdrExposure = 1.0f;
};