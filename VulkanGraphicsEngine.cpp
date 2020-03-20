#include "vkApp.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	//Initialize
	int windowWidth = 1024;
	int windowHeight = 760;

	MSG msg = {};
	HWND hWnd = nullptr;

	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_GLOBALCLASS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszClassName = L"MainWindowClass";
	RegisterClassEx(&wcex);

	hWnd = CreateWindowW(L"MainWindowClass", L"Base 3D Project", WS_OVERLAPPED | WS_SYSMENU, CW_USEDEFAULT, 0, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	VkApp vkApp;
	vkApp.Initialize(windowWidth, windowHeight, hWnd, hInstance);

	vkApp.Start();

	//Update
	HANDLE phWait = CreateWaitableTimer(NULL, FALSE, NULL);
	LARGE_INTEGER liDueTime = {};
	liDueTime.QuadPart = -1i64;
	SetWaitableTimer(phWait, &liDueTime, 1, NULL, NULL, 0);

	DWORD dwRet = 0;
	BOOL bExit = FALSE;
	while (!bExit)
	{
		dwRet = MsgWaitForMultipleObjects(1, &phWait, FALSE, INFINITE, QS_ALLINPUT);
		switch (dwRet - WAIT_OBJECT_0)
		{
		case 0:
		case WAIT_TIMEOUT:
		{
			vkApp.Loop();
		}

		break;
		case 1:
		{
			while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (WM_QUIT != msg.message)
				{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
				else
				{
					bExit = TRUE;
				}
			}
		}
		break;
		default:
			break;
		}
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}