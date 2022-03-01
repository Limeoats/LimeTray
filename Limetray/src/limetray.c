#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <assert.h>

#include "../resource.h"

#define NOTIFICATION_TRAY_ICON_MSG (WM_USER + 0x100)
#define MENU_EXIT 0x01
#define MENU_COPY 0x02

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
	const wchar_t* tooltip_text = L"LimeTray";
	wcscpy_s(nid.szTip, wcslen(tooltip_text) + 1, tooltip_text);
	nid.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDB_PNG1), IMAGE_ICON, 32, 32, LR_SHARED);
	nid.uCallbackMessage = NOTIFICATION_TRAY_ICON_MSG;

	if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
		MessageBox(NULL, L"Tray icon creation failed.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (ret == -1) {
			MessageBox(NULL, L"Error getting message.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
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
	case NOTIFICATION_TRAY_ICON_MSG:
	{
		switch (LOWORD(lParam)) {
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN:
		{
			HMENU popup_menu = CreatePopupMenu();
			POINT click_point;
			GetCursorPos(&click_point);

			InsertMenu(popup_menu, 0, MF_BYPOSITION | MF_STRING, MENU_EXIT, L"Exit");
			InsertMenu(popup_menu, 0, MF_BYPOSITION | MF_STRING, MENU_COPY, L"Copy something");
			SetForegroundWindow(hWnd);
			TrackPopupMenu(popup_menu, TPM_LEFTALIGN | TPM_TOPALIGN, click_point.x - 32, click_point.y - 32 - (2 * 10), 0, hWnd, NULL);
		} break;
		}

	} break;
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == MENU_EXIT) {
			PostQuitMessage(0);
		}
		else if (LOWORD(wParam) == MENU_COPY) {
			const char text[1000] = "this is my test password that I want to copy..!";
			const size_t len = sizeof(text);
			HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, len);
			assert(mem != NULL);
			
			LPVOID s = GlobalLock(mem);
			assert(s != NULL);
				
			memcpy(GlobalLock(mem), text, len);
			GlobalUnlock(mem);
			OpenClipboard(0);
			EmptyClipboard();
			SetClipboardData(CF_TEXT, mem);
			CloseClipboard();
		}
	} break;
	default:
	{
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
	}
	return 0;
}