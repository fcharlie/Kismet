#ifndef NEONWINDOW_H
#define NEONWINDOW_H
#pragma once
#include <atlbase.h>
#include <atlwin.h>
#include <atlctl.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <vector>
#include <string>

#define NEONWINDOWNAME L"Neon.UI.Window"

#define NEON_WINDOW_CLASSSTYLE WS_OVERLAPPED | WS_SYSMENU | \
WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS&~WS_MAXIMIZEBOX
typedef CWinTraits<NEON_WINDOW_CLASSSTYLE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE> CMetroWindowTraits;

class NeonWindow :public CWindowImpl<NeonWindow, CWindow, CMetroWindowTraits> {
private:
public:
	NeonWindow(const NeonWindow &) = delete;
	NeonWindow &operator=(const NeonWindow &) = delete;
	NeonWindow();
	~NeonWindow();
	LRESULT InitializeWindow();
	DECLARE_WND_CLASS(NEONWINDOWNAME)
	BEGIN_MSG_MAP(MetroWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()
	LRESULT OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
	LRESULT OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
	LRESULT OnClose(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
};

#endif