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

#include "limetray.h"

#include <prsht.h>
#include <commctrl.h>
#include <stdio.h>
#include <assert.h>
#include <stdio.h>

#include "../resource.h"

#define NOTIFICATION_TRAY_ICON_MSG (WM_USER + 0x100)
#define MENU_EXIT 0x01
#define MENU_SETTINGS 0x02

#define MENU_COPY_TEXT 0x03

#define SETTINGS_TAB_COPY 0x10

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcSettings(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define COPY_STRING_MAX_ROWS 32

app_t* g_app;


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

static HWND create_settings_tabs(HWND hwnd_parent)
{
	RECT client_rect;
	TCITEM tab_item;
	INITCOMMONCONTROLSEX ctrls;
	TCHAR buffer[256];
	
	ctrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ctrls.dwICC = ICC_TAB_CLASSES;

	InitCommonControlsEx(&ctrls);

	GetClientRect(hwnd_parent, &client_rect);
	g_app->hwnd_settings_tabs = CreateWindow(WC_TABCONTROL, L"", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
		0, 0, client_rect.right, client_rect.bottom, hwnd_parent, NULL, g_app->hInst, NULL);
	if (g_app->hwnd_settings_tabs == NULL) {
		return NULL;
	}

	tab_item.mask = TCIF_TEXT | TCIF_IMAGE;
	tab_item.iImage = -1;
	tab_item.pszText = buffer;

	wsprintf(buffer, L"Copy strings");
	if (TabCtrl_InsertItem(g_app->hwnd_settings_tabs, SETTINGS_TAB_COPY, &tab_item) == -1) {
		DestroyWindow(g_app->hwnd_settings_tabs);
		return NULL;
	}


	wsprintf(buffer, L"Something else");
	if (TabCtrl_InsertItem(g_app->hwnd_settings_tabs, SETTINGS_TAB_COPY + 0x01, &tab_item) == -1) {
		DestroyWindow(g_app->hwnd_settings_tabs);
		return NULL;
	}
	return g_app->hwnd_settings_tabs;
}

static void setup_copy_strings_settings()
{
    g_app->copy_strings_rows = calloc(COPY_STRING_MAX_ROWS, sizeof(copy_strings_row_t));
	int n_rows = 1;
	for (int i = 0; i < n_rows; ++i) 
	{
        TCHAR key1[10];
        TCHAR key2[10];
        sprintf(key1, "k%d", i);
        sprintf(key2, "v%d", i);
		HWND key_textbox = CreateWindowEx(WS_EX_CLIENTEDGE, L"edit_key", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 
            100, 100, 50, 40, g_app->hwnd_settings_tabs, NULL, g_app->hInst, NULL);
        HWND value_textbox = CreateWindowEx(WS_EX_CLIENTEDGE, L"edit_value", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 
            200, 200, 50, 40, g_app->hwnd_settings_tabs, NULL, g_app->hInst, NULL);
        copy_strings_row_t row = {
            .hwnd_key_textbox = key_textbox,
            .hwnd_value_textbox = value_textbox
        };
        g_app->copy_strings_rows[g_app->copy_string_index++] = row;
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
	g_app = malloc(sizeof(app_t));
    g_app->copy_string_index = 0;

	MSG msg;
	BOOL ret;
	WNDCLASSEX wc;
	WNDCLASSEX wc_settings;

	g_app->hInst = hInstance;


	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g_app->hInst;
	wc.hIcon = (HICON)LoadImage(g_app->hInst, MAKEINTRESOURCE(IDB_PNG1), IMAGE_ICON, 32, 32, LR_SHARED);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"LimeTray";
	wc.hIconSm = (HICON)LoadImage(g_app->hInst, MAKEINTRESOURCE(IDB_PNG1), IMAGE_ICON, 32, 32, LR_SHARED);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, L"Window registration failed.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	g_app->hWnd = CreateWindowEx(
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

	if (g_app->hWnd == NULL) {
		MessageBox(NULL, L"Window creation failed.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	// Initialize Settings window
	wc_settings.cbSize = sizeof(WNDCLASSEX);
	wc_settings.style = CS_VREDRAW | CS_HREDRAW;
	wc_settings.lpfnWndProc = WndProcSettings;
	wc_settings.cbClsExtra = 0;
	wc_settings.cbWndExtra = 0;
	wc_settings.hInstance = g_app->hInst;
	wc_settings.hIcon = (HICON)LoadImage(g_app->hInst, MAKEINTRESOURCE(IDB_PNG1), IMAGE_ICON, 32, 32, LR_SHARED);
	wc_settings.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc_settings.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc_settings.lpszMenuName = NULL;
	wc_settings.lpszClassName = L"LimeTray Settings";
	wc_settings.hIconSm = (HICON)LoadImage(g_app->hInst, MAKEINTRESOURCE(IDB_PNG1), IMAGE_ICON, 32, 32, LR_SHARED);

	if (!RegisterClassEx(&wc_settings)) {
		MessageBox(NULL, L"Settings window registration failed.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	RECT center = get_center_of_window(GetDesktopWindow(), 640, 480);

	g_app->hWndSettings = CreateWindowEx(
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
		g_app->hInst,
		NULL
	);

	HWND hwnd_tabs = create_settings_tabs(g_app->hWndSettings);
	setup_copy_strings_settings();

	g_app->nid.cbSize = sizeof(NOTIFYICONDATA);
	g_app->nid.hWnd = g_app->hWnd;
	g_app->nid.uID = IDB_PNG1;
	g_app->nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	const wchar_t* tooltip_text = L"LimeTray";
	wcscpy_s(g_app->nid.szTip, wcslen(tooltip_text) + 1, tooltip_text);
	g_app->nid.hIcon = (HICON)LoadImage(g_app->hInst, MAKEINTRESOURCE(IDB_PNG1), IMAGE_ICON, 32, 32, LR_SHARED);
	g_app->nid.uCallbackMessage = NOTIFICATION_TRAY_ICON_MSG;

	if (!Shell_NotifyIcon(NIM_ADD, &g_app->nid)) {
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
			free(g_app);
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
			ShowWindow(g_app->hWndSettings, SW_SHOW);
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
	case WM_NOTIFY:
	{
		switch(((LPNMHDR)lParam)->code) 
		{
			case TCN_SELCHANGING:
			{
				return FALSE;
			} break;
			case TCN_SELCHANGE:
			{
				int tab_id = TabCtrl_GetCurSel(g_app->hwnd_settings_tabs);
				switch (tab_id)
				{
					// 0 is the Copy strings section
					case 0:
					{
                        for (int i = 0; i < g_app->copy_string_index; ++i)
                        {
                            ShowWindow(g_app->copy_strings_rows[i].hwnd_key_textbox, SW_SHOW);
                            ShowWindow(g_app->copy_strings_rows[i].hwnd_value_textbox, SW_SHOW);
                        }
					}
				}
			} break;
		}
	} break;
	default:
	{
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
	}
	return 0;
}
