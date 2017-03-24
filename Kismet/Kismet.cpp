// Kismet.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Kismet.h"
#include <CommCtrl.h> 
#include <commdlg.h>
#include "NeonWindow.h"

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
