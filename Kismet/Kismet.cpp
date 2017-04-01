// Kismet.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Kismet.h"
#include <CommCtrl.h> 
#include <commdlg.h>
#include <regex>
#include <wchar.h>
#include <PathCch.h>
#include "NeonWindow.h"

/// rgb(12,54,152) rgb to hex
bool RgbColorValue(const std::wstring &va,std::uint32_t &color) {
	unsigned r=0, g=0, b=0;
	if (_snwscanf_s(va.data(), va.size(), LR"(rgb(%d,%d,%d)", &r, &g, &b) != 3)
		return false;
	color = (r << 16) + (g << 8) + b;
	return true;
}

/// LR"(rgb\(\w+,\w+,\w+\))"


bool InitializeColorValue(const std::wstring &cs,std::uint32_t &va) {
	if (cs.front() == L'#') {
		wchar_t *c = nullptr;
		va= wcstol(cs.data()+1, &c, 16);
		return true;
	}
	else if (cs.front() == L'r') {
		return RgbColorValue(cs.data(), va);
	}
	return false; 
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

std::wstring PathCombineWithExe(const std::wstring &file) {
	std::wstring path(PATHCCH_MAX_CCH, L'\0');
	auto N=GetModuleFileNameW(nullptr, &path[0], path.size());
	path.resize(N);
	auto buf = &path[0];
	if (!PathRemoveFileSpecW(buf)) {
		return L"";
	}
	path.resize(wcslen(buf));
	path.append(L"\\").append(file);
	return path;
}

bool ApplyWindowSettings(NeonSettings &ns) {
	auto file = PathCombineWithExe(L"kismet.ini");
	if (!PathFileExistsW(file.data()))
		return false;
	wchar_t colorbuf[20];
	swprintf_s(colorbuf, L"#%06X", ns.bkcolor);
	WritePrivateProfileStringW(L"Window", L"Background", colorbuf, file.data());
	swprintf_s(colorbuf, L"#%06X", ns.fgcolor);
	WritePrivateProfileStringW(L"Window", L"Foreground", colorbuf, file.data());
	return true;
}

bool UpdateWindowSettings(NeonSettings &ns) {
	auto file = PathCombineWithExe(L"kismet.ini");
	if (file.empty())
		return false;
	if (!PathFileExistsW(file.data()))
		return false;
	wchar_t colorbuf[20];
	///GetPrivateProfileStringW()
	return true;
}

int NeonWindowLoop()
{
	INITCOMMONCONTROLSEX info = { sizeof(INITCOMMONCONTROLSEX),
		ICC_TREEVIEW_CLASSES | ICC_COOL_CLASSES | ICC_LISTVIEW_CLASSES };
	InitCommonControlsEx(&info);
	NeonWindow window;
	MSG msg;
	UpdateWindowSettings(window.Settings());
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
