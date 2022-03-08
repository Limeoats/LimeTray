#include <windows.h>
#include <shellapi.h>

typedef struct {
    HWND hwnd_key_textbox;
    HWND hwnd_value_textbox;
} copy_strings_row_t;

typedef struct {
    HWND hWnd;	
    HWND hWndSettings;
    HINSTANCE hInst;
    NOTIFYICONDATA nid;

    HWND hwnd_settings_tabs;

    HWND hwnd_copy_strings_tab;
    copy_strings_row_t* copy_strings_rows;
    int copy_string_index;
} app_t;