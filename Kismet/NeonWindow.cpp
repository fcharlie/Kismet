#include "stdafx.h"
#include "NeonWindow.h"
#include <Prsht.h>
#include <CommCtrl.h>
#include <Shlwapi.h>
#include <Shellapi.h>
#include <PathCch.h>
#include <Mmsystem.h>

NeonWindow::NeonWindow()
{
}

NeonWindow::~NeonWindow()
{
}
#define WS_NORESIZEWINDOW (WS_OVERLAPPED | WS_CAPTION |WS_SYSMENU | \
 WS_CLIPCHILDREN | WS_MINIMIZEBOX )

LRESULT NeonWindow::InitializeWindow()
{
	HRESULT  hr = E_FAIL;
	RECT layout = { 100, 100,1000, 640 };
	Create(nullptr, layout, L"Kismet Neon",
		WS_NORESIZEWINDOW,
		WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
	return S_OK;
}

LRESULT NeonWindow::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandle)
{
	HICON hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(107));
	SetIcon(hIcon, TRUE);
	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
	ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
	ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
	::DragAcceptFiles(m_hWnd, TRUE);
	return S_OK;
}

LRESULT NeonWindow::OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandle)
{
	PostQuitMessage(0);
	return S_OK;
}

LRESULT NeonWindow::OnClose(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandle)
{
	::DestroyWindow(m_hWnd);
	return S_OK;
}
