/*
	TODO:
	- Settings window that uses Propery Sheets for the tabs
	- Copy settings tab
		- Editable list view with two columns (name and string to copy)
		- Button to add an extra row
			- https://stackoverflow.com/questions/3217362/adding-items-to-a-listview
		- Save button that will write the data to settings.cfg
	- Update the Copy submenu to build its items based on the copy settings
*/

#ifndef UNICODE
#define UNICODE
#endif

#pragma warning(disable: 6001)

#include <windows.h>
#include <shellapi.h>
#include <prsht.h>
#include <stdio.h>
#include <assert.h>
#include <stdio.h>

#include "../resource.h"

#define NOTIFICATION_TRAY_ICON_MSG (WM_USER + 0x100)
#define MENU_EXIT 0x01
#define MENU_SETTINGS 0x02

#define MENU_COPY_TEXT 0x03

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcSettings(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hWnd;	
HWND hWndSettings;
NOTIFYICONDATA nid = { 0 };

PROPSHEETPAGE prop_sheet_pages[1];
PROPSHEETHEADER prop_sheet_header;

static void copy_text_to_clipboard(const char* text, int len)
{
	HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, len);
	assert(mem != NULL);

	LPVOID s = GlobalLock(mem);
	assert(s != NULL);

	memcpy(s, text, len);
	GlobalUnlock(mem);
	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, mem);
	CloseClipboard();
}

static BOOL file_exists(LPCTSTR path) {
	DWORD a = GetFileAttributes(path);
	return (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY));
}

static RECT get_center_of_window(HWND parent_window, int width, int height) {
	RECT rect;
	GetClientRect(parent_window, &rect);
	rect.left = (rect.right / 2) - (width / 2);
	rect.top = (rect.bottom / 2) - (height / 2);
	return rect;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	BOOL ret;
	WNDCLASSEX wc;
	WNDCLASSEX wc_settings;

	InitCommonControlsEx();


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

	// Initialize Settings window
	wc_settings.cbSize = sizeof(WNDCLASSEX);
	wc_settings.style = CS_VREDRAW | CS_HREDRAW;
	wc_settings.lpfnWndProc = WndProcSettings;
	wc_settings.cbClsExtra = 0;
	wc_settings.cbWndExtra = 0;
	wc_settings.hInstance = hInstance;
	wc_settings.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDB_PNG1), IMAGE_ICON, 32, 32, LR_SHARED);
	wc_settings.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc_settings.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc_settings.lpszMenuName = NULL;
	wc_settings.lpszClassName = L"LimeTray Settings";
	wc_settings.hIconSm = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDB_PNG1), IMAGE_ICON, 32, 32, LR_SHARED);

	if (!RegisterClassEx(&wc_settings)) {
		MessageBox(NULL, L"Settings window registration failed.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	RECT center = get_center_of_window(GetDesktopWindow(), 640, 480);

	hWndSettings = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"LimeTray Settings",
		L"LimeTray Settings",
		WS_OVERLAPPEDWINDOW,
		center.left,
		center.top,
		640,
		480,
		NULL,
		NULL,
		hInstance,
		NULL
	);


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
			HMENU copy_submenu = CreateMenu();

			POINT click_point;
			GetCursorPos(&click_point);

			AppendMenu(popup_menu, MF_POPUP | MF_STRING, (UINT_PTR)copy_submenu, L"Copy ");
			AppendMenu(copy_submenu, MF_STRING | MF_POPUP, MENU_COPY_TEXT, L"Personify cloud");
			AppendMenu(popup_menu, MF_POPUP | MF_STRING, MENU_SETTINGS, L"Settings");
			AppendMenu(popup_menu, MF_BYPOSITION | MF_STRING, MENU_EXIT, L"Exit");

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
		else if (LOWORD(wParam) == MENU_COPY_TEXT) {
			char buffer[1000] = "personify cloud password copied..!";
			copy_text_to_clipboard(buffer, sizeof(buffer));
		}
		else if (LOWORD(wParam) == MENU_SETTINGS) {
			/*TCHAR buffer[1000];
			GetCurrentDirectory(sizeof(buffer) / sizeof(TCHAR), buffer);*/
			if (!file_exists(L"settings.cfg")) {
				CreateFile(L"settings.cfg", GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			}
			ShowWindow(hWndSettings, SW_SHOW);
		}
	} break;
	default:
	{
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
	}
	return 0;
}

LRESULT CALLBACK WndProcSettings(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE:
	{
	} break;
	case WM_COMMAND:
	{
	} break;
	case WM_CLOSE:
	{
		ShowWindow(hWnd, SW_HIDE);
	} break;
	default:
	{
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
	}
	return 0;
}