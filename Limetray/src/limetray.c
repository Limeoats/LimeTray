#include <windows.h>
#include <shellapi.h>
#include <stdio.h>

#include "../resource.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hWnd;
NOTIFYICONDATA nid = { 0 };

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	BOOL ret;
	WNDCLASSEX wc;


	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDB_PNG1), IMAGE_ICON, 32, 32, LR_SHARED);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"LimeTray";
	wc.hIconSm = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDB_PNG1), IMAGE_ICON, 32, 32, LR_SHARED);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, L"Window registration failed.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"LimeTray",
		L"LimeTray",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (hWnd == NULL) {
		MessageBox(NULL, L"Window creation failed.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = IDB_PNG1;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	char* tooltip_text = L"LimeTray";
	wcscpy_s(nid.szTip, wcslen(tooltip_text) + 1, tooltip_text);
	nid.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDB_PNG1), IMAGE_ICON, 32, 32, LR_SHARED);
	nid.uCallbackMessage = WM_USER + 1;

	if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
		MessageBox(NULL, L"Tray icon creation failed.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (ret == -1) {
			fprintf(stderr, "Error getting message.\n");
			exit(1);
		}
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE:
	{
	} break;
	default:
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	}
	return 0;
}