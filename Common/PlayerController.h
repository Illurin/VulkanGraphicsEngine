#pragma once
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

class PlayerInput {
public:
	//初始化函数
	bool Init(HINSTANCE hInstance, HWND hwnd);
	bool Update();
	
	//获得键盘信息函数
	bool GetKey(int dikValue);
	bool GetKeyDown(int dikValue);

	//获得鼠标信息函数
	bool GetMouse(int button);
	bool GetMouseDown(int button);
	void GetCursorPosition(long& cursorPositionX, long& cursorPositionY);

private:
	IDirectInput8* inputSystem;
	IDirectInputDevice8* keyboardDevice;
	IDirectInputDevice8* mouseDevice;

	char currentKeyBuffer[256] = {};
	char lastKeyBuffer[256] = {};

	DIMOUSESTATE currentMouseState;
	DIMOUSESTATE lastMouseState;

	HWND hwnd;
};