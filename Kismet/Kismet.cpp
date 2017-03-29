// Kismet.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Kismet.h"
#include <CommCtrl.h> 
#include <commdlg.h>
#include <regex>
#include <wchar.h>
#include "NeonWindow.h"

/// rgb(12,54,152)
bool MatchColorValue(const std::wstring &va,std::uint32_t &color) {
	unsigned r, g, b;
	_snwscanf(va.data(), va.size(), LR"(rgb(%d,%d,%d)", &r, &g, &b);
	wchar_t buf[16];
	_snwprintf_s(buf, sizeof(buf), L"%02x%02x%02x", r, g, b);
	wchar_t *c=nullptr;
	color=wcstol(buf, &c, 16);
	return true;
}

/// LR"(rgb\(\w+,\w+,\w+\))"
std::uint32_t InitializeColorValue(const std::wstring &cs) {
	if (cs.front() == L'#') {
		wchar_t *c = nullptr;
		return wcstol(cs.data()+1, &c, 16);
	}
	else if (cs.front() == L'r') {
		std::uint32_t va;
		MatchColorValue(cs.data(), va);
		return va;
	}
	return UINT32_MAX; ///invalid color
}

class DotComInitialize {
public:
	DotComInitialize()
	{
		CoInitialize(NULL);
	}
	~DotComInitialize()
	{
		CoUninitialize();
	}
};

int NeonWindowLoop()
{
	INITCOMMONCONTROLSEX info = { sizeof(INITCOMMONCONTROLSEX),
		ICC_TREEVIEW_CLASSES | ICC_COOL_CLASSES | ICC_LISTVIEW_CLASSES };
	InitCommonControlsEx(&info);
	NeonWindow window;
	MSG msg;
	window.InitializeWindow();
	window.ShowWindow(SW_SHOW);
	window.UpdateWindow();
	while (GetMessage(&msg, nullptr, 0, 0)>0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	DotComInitialize dot;
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
    return NeonWindowLoop();
}
