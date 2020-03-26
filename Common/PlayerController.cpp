#include "PlayerController.h"

bool PlayerInput::Init(HINSTANCE hInstance, HWND hwnd) {
	this->hwnd = hwnd;

	if(FAILED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&inputSystem, NULL)))
		return false;
	if (FAILED(inputSystem->CreateDevice(GUID_SysKeyboard, &keyboardDevice, NULL)))
		return false;
	if (FAILED(keyboardDevice->SetDataFormat(&c_dfDIKeyboard)))
		return false;
	if (FAILED(keyboardDevice->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
		return false;
	if (FAILED(inputSystem->CreateDevice(GUID_SysMouse, &mouseDevice, NULL)))
		return false;
	if (FAILED(mouseDevice->SetDataFormat(&c_dfDIMouse)))
		return false;
	if (FAILED(mouseDevice->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
		return false;

	return true;
}

bool PlayerInput::Update() {
	memcpy(lastKeyBuffer, currentKeyBuffer, sizeof(char) * 256);
	
	if(FAILED(keyboardDevice->Acquire()))
		return false;
	if (FAILED(keyboardDevice->GetDeviceState(sizeof(char) * 256, currentKeyBuffer)))
		return false;

	if (FAILED(mouseDevice->Acquire()))
		return false;
	if (FAILED(mouseDevice->GetDeviceState(sizeof(currentMouseState), (LPVOID)&currentMouseState)))
		return false;

	return true;
}

bool PlayerInput::GetKey(int dikVaule) {
	if (currentKeyBuffer[dikVaule] & 0x80)
		return true;
	return false;
}

bool PlayerInput::GetKeyDown(int dikValue) {
	if ((currentKeyBuffer[dikValue] & 0x80) && !(lastKeyBuffer[dikValue] & 0x80))
		return true;
	return false;
}

bool PlayerInput::GetMouse(int button) {
	if (currentMouseState.rgbButtons[button] & 0x80)
		return true;
	return false;
}

bool PlayerInput::GetMouseDown(int button) {
	if ((currentMouseState.rgbButtons[button] & 0x80) && !(lastMouseState.rgbButtons[button] & 0x80))
		return true;
	return false;
}

void PlayerInput::GetCursorPosition(long& cursorPositionX, long& cursorPositionY) {
	POINT initialCursorPos;
	GetCursorPos(&initialCursorPos);

	RECT windowRect;
	GetWindowRect(hwnd, &windowRect);

	cursorPositionX = initialCursorPos.x - windowRect.left - 10;
	cursorPositionY = initialCursorPos.y - windowRect.top - 30;
}