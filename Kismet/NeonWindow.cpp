#include "stdafx.h"
#include "NeonWindow.h"
#include "Kismet.h"
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include <Prsht.h>
#include <CommCtrl.h>
#include <Shlwapi.h>
#include <Shellapi.h>
#include <PathCch.h>
#include <Windowsx.h>
#include <Mmsystem.h>

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

template<class Interface>
inline void
SafeRelease(
	Interface **ppInterfaceToRelease
)
{
	if (*ppInterfaceToRelease != NULL) {
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = NULL;
	}
}

NeonWindow::NeonWindow()
	:m_pFactory(nullptr),
	m_pBackgroundBrush(nullptr),
	m_pNormalTextBrush(nullptr),
	m_pHwndRenderTarget(nullptr),
	m_pWriteFactory(nullptr),
	m_pWriteTextFormat(nullptr)
{
}

NeonWindow::~NeonWindow()
{
	SafeRelease(&m_pWriteTextFormat);
	SafeRelease(&m_pWriteFactory);
	SafeRelease(&m_pBackgroundBrush);
	SafeRelease(&m_pNormalTextBrush);
	SafeRelease(&m_pHwndRenderTarget);
	SafeRelease(&m_pFactory);
}
#define WS_NORESIZEWINDOW (WS_OVERLAPPED | WS_CAPTION |WS_SYSMENU | \
 WS_CLIPCHILDREN | WS_MINIMIZEBOX )

LRESULT NeonWindow::InitializeWindow()
{
	HRESULT  hr = E_FAIL;
	RECT layout = { 100, 100,800, 470 };
	title.assign(L"Kismet Neon");
	Create(nullptr, layout, title.data(),
		WS_NORESIZEWINDOW,
		WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
	Securehashsum::Instance().InitializeSecureTask();
	return S_OK;
}

/////
HRESULT NeonWindow::CreateDeviceIndependentResources() {
	HRESULT hr = S_OK;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
	if (SUCCEEDED(hr)) {
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_pWriteFactory));
		if (SUCCEEDED(hr)) {
			hr = m_pWriteFactory->CreateTextFormat(
				L"Segoe UI",
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				12.0f * 96.0f / 72.0f,
				L"zh-CN",
				&m_pWriteTextFormat);
		}

	}
	return hr;
}
HRESULT NeonWindow::Initialize() {
	auto hr = CreateDeviceIndependentResources();
	FLOAT dpiX, dpiY;
	m_pFactory->GetDesktopDpi(&dpiX, &dpiY);
	return hr;
}
HRESULT NeonWindow::CreateDeviceResources() {
	HRESULT hr = S_OK;

	if (!m_pHwndRenderTarget) {
		RECT rc;
		::GetClientRect(m_hWnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU(
			rc.right - rc.left,
			rc.bottom - rc.top
		);
		hr = m_pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hWnd, size),
			&m_pHwndRenderTarget
		);
		if (SUCCEEDED(hr)) {
			////
			hr = m_pHwndRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF((UINT32)ns.bkcolor),
				&m_pBackgroundBrush
			);
		}
		if (SUCCEEDED(hr)) {
			hr = m_pHwndRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Black),
				&m_pNormalTextBrush
			);
		}
	}
	return hr;
}
void NeonWindow::DiscardDeviceResources() {
	SafeRelease(&m_pBackgroundBrush);
	SafeRelease(&m_pNormalTextBrush);
}
HRESULT NeonWindow::OnRender() {
	auto hr = CreateDeviceResources();
	if (SUCCEEDED(hr)) {
#pragma warning(disable:4244)
#pragma warning(disable:4267)
		if (SUCCEEDED(hr)) {
			auto size = m_pHwndRenderTarget->GetSize();
			m_pHwndRenderTarget->BeginDraw();
			m_pHwndRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			m_pHwndRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
			m_pHwndRenderTarget->FillRectangle(
				D2D1::RectF(0,250,800.0,480.0),
				m_pBackgroundBrush);
			//// write progress 100 %
			if (progress > 0 && progress <= 100) {
				auto progrssText=std::to_wstring(progress) + L"%";
				m_pHwndRenderTarget->DrawTextW(
					progrssText.c_str(), progrssText.size(), m_pWriteTextFormat,
					D2D1::RectF(160.0f, 278.0f,250.0f,305.0f),
					m_pNormalTextBrush, D2D1_DRAW_TEXT_OPTIONS_NONE,
					DWRITE_MEASURING_MODE_NATURAL);
			}
			hr = m_pHwndRenderTarget->EndDraw();
		}
		if (hr == D2DERR_RECREATE_TARGET) {
			hr = S_OK;
			DiscardDeviceResources();
			//::InvalidateRect(m_hWnd, nullptr, FALSE);
		}
#pragma warning(default:4244)
#pragma warning(default:4267)	
	}
	return hr;
}
D2D1_SIZE_U NeonWindow::CalculateD2DWindowSize() {
	RECT rc;
	::GetClientRect(m_hWnd, &rc);

	D2D1_SIZE_U d2dWindowSize = { 0 };
	d2dWindowSize.width = rc.right;
	d2dWindowSize.height = rc.bottom;

	return d2dWindowSize;
}

void NeonWindow::OnResize(
	UINT width,
	UINT height
)
{
	if (m_pHwndRenderTarget) {
		m_pHwndRenderTarget->Resize(D2D1::SizeU(width, height));
	}
}

void NeonWindow::UpdateTitle(const std::wstring & title_)
{
	std::wstring xtitle = title;
	if (!title_.empty()) {
		xtitle.append(L" - ").append(title_);
	}
	SetWindowTextW(xtitle.data());
}



bool NeonWindow::FilesumInvoke(std::int32_t state, std::uint32_t pg, std::wstring data)
{
	switch (state) {
	case kFilesumBroken:
		return false;
	case kFilesumMessage:
		content.assign(std::move(data));
		::SetWindowTextW(hContent, content.data());
		return true;
	case kFilesumCompleted:
		if (Button_GetCheck(hCheck) == BST_CHECKED) {
			std::transform(data.begin(), data.end(), data.begin(), std::toupper);
		}
		content.append(data);
		::SetWindowTextW(hContent, content.data());
		break;
	case kFilesumCollCompleted:
		if (Button_GetCheck(hCheck) == BST_CHECKED) {
			std::transform(data.begin(), data.end(), data.begin(), std::toupper);
		}
		content.append(data).append(L" *coll*");
		::SetWindowTextW(hContent, content.data());
		break;
	case kFilesumProgress:
		progress = pg;
		InvalidateRect(nullptr, false);
		//InvalidateRect(&progressRect, false);
		return true;
	}
	return true;
}


//////////////////////////
inline bool InitializeComboHash(HWND hWnd) {
	const wchar_t *hash[] = {
		L"MD5",
		L"SHA1",
		L"SHA1 - Coll",
		L"SHA224",
		L"SHA256",
		L"SHA384",
		L"SHA512",
		L"SHA3 - 224",
		L"SHA3 - 256",
		L"SHA3 - 384",
		L"SHA3 - 512",
	};
	auto N = ARRAYSIZE(hash) - 3;
	for (auto c : hash) {
		::SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)c);
	}
	::SendMessage(hWnd, CB_SETCURSEL, N > 0 ? N : 0, 0);
	return true;
}

inline bool ComboHashValue(int i, FilesumEm &fse) {
	switch (i) {
	case 0:
		fse.alm = kFilesumMD5;
		break;
	case 1:
		fse.alm = kFilesumSHA1;
		break;
	case 2:
		fse.alm = kFilesumSHA1DC;
		break;
	case 3:
		fse.alm = kFilesumSHA2;
		fse.width = 224;
		break;
	case 4:
		fse.alm = kFilesumSHA2;
		fse.width = 256;
		break;
	case 5:
		fse.alm = kFilesumSHA2;
		fse.width = 384;
		break;
	case 6:
		fse.alm = kFilesumSHA2;
		fse.width = 512;
		break;
	case 7:
		fse.alm = kFilesumSHA3;
		fse.width = 224;
		break;
	case 8:
		fse.alm = kFilesumSHA3;
		fse.width = 256;
		break;
	case 9:
		fse.alm = kFilesumSHA3;
		fse.width = 384;
		break;
	case 10:
		fse.alm = kFilesumSHA3;
		fse.width = 512;
		break;
	default:
		return false;
	}
	return true;
}


#define WINDOWEXSTYLE WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY
#define EDITBOXSTYLE  WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_VISIBLE |WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL
#define PUSHBUTTONSTYLE BS_PUSHBUTTON | BS_TEXT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE
#define CHECKBOXSTYLE BS_PUSHBUTTON | BS_TEXT | BS_DEFPUSHBUTTON | BS_CHECKBOX | BS_AUTOCHECKBOX | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE
#define COMBOBOXSTYLE WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST  | CBS_HASSTRINGS

LRESULT NeonWindow::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandle)
{
	auto hr = Initialize();
	if (hr != S_OK) {
		::MessageBoxW(nullptr, L"Initialize() failed", L"Fatal error", MB_OK | MB_ICONSTOP);
		std::terminate();
		return S_FALSE;
	}
	HICON hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(107));
	SetIcon(hIcon, TRUE);
	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
	ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
	ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
	::DragAcceptFiles(m_hWnd, TRUE);
	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	LOGFONT logFont = { 0 };
	GetObjectW(hFont, sizeof(logFont), &logFont);
	DeleteObject(hFont);
	hFont = NULL;

	logFont.lfHeight = 20;
	logFont.lfWeight = FW_NORMAL;
	wcscpy_s(logFont.lfFaceName, L"Segoe UI");
	hFont = CreateFontIndirectW(&logFont);
	auto LambdaCreateWindow = [&](LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle,
		int X, int Y, int nWidth, int nHeight, HMENU hMenu)->HWND {
		auto hw = CreateWindowExW(WINDOWEXSTYLE, lpClassName, lpWindowName,
			dwStyle, X, Y, nWidth, nHeight, m_hWnd, hMenu, HINST_THISCOMPONENT, nullptr);
		if (hw) {
			::SendMessageW(hw, WM_SETFONT, (WPARAM)hFont, lParam);
		}
		return hw;
	};
	auto LambdaCreateWindowEdge = [&](LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle,
		int X, int Y, int nWidth, int nHeight, HMENU hMenu)->HWND {
		auto hw = CreateWindowExW(WINDOWEXSTYLE | WS_EX_CLIENTEDGE, lpClassName, lpWindowName,
			dwStyle, X, Y, nWidth, nHeight, m_hWnd, hMenu, HINST_THISCOMPONENT, nullptr);
		if (hw) {
			::SendMessageW(hw, WM_SETFONT, (WPARAM)hFont, lParam);
		}
		return hw;
	};
	RECT rect;
	GetWindowRect(&rect);
	hContent = LambdaCreateWindow(WC_EDITW, L"",
		EDITBOXSTYLE |ES_MULTILINE| ES_READONLY,
		20, 0, rect.right-rect.left-60, 250,
		HMENU(IDC_CONTENT_EDIT));

	hCheck = LambdaCreateWindow(WC_BUTTONW, 
		L"Uppercase", 
		CHECKBOXSTYLE, 
		20, 275, 90, 27, nullptr);

	hCombo = LambdaCreateWindow(WC_COMBOBOXW, L"",
		COMBOBOXSTYLE,
		rect.right - rect.left - 420, 275, 120, 27, nullptr);

	hOpenButton = LambdaCreateWindow(WC_BUTTONW,
		L"Clear",
		PUSHBUTTONSTYLE,
		rect.right - rect.left - 290, 275, 120, 27,
		HMENU(IDC_CLEAR_BUTTON));

	hOpenButton = LambdaCreateWindow(WC_BUTTONW, 
		L"Open", 
		PUSHBUTTONSTYLE, 
		rect.right-rect.left-160, 275, 120, 27, 
		HMENU(IDC_FILEOPEN_BUTTON));

	InitializeComboHash(hCombo);
	progressRect = { 160, 278,250,305};
	hBrush = CreateSolidBrush(ColorConvert(ns.bkcolor));
	hBrushContent = CreateSolidBrush(ColorConvert(ns.fgcolor));
	return S_OK;
}

LRESULT NeonWindow::OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandle)
{
	if (hBrush) {
		DeleteObject(hBrush);
	}
	if (hBrushContent) {
		DeleteObject(hBrushContent);
	}
	PostQuitMessage(0);
	return S_OK;
}

LRESULT NeonWindow::OnClose(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandle)
{
	::DestroyWindow(m_hWnd);
	return S_OK;
}

LRESULT NeonWindow::OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandle)
{
	UINT width = LOWORD(lParam);
	UINT height = HIWORD(lParam);
	OnResize(width, height);
	return S_OK;
}

LRESULT NeonWindow::OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandle)
{
	OnRender();
	ValidateRect(NULL);
	return S_OK;
}

LRESULT NeonWindow::OnDisplayChange(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	::InvalidateRect(m_hWnd, NULL, FALSE);
	return S_OK;
}

LRESULT NeonWindow::OnColorStatic(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandle)
{
	HDC hdc = (HDC)wParam;
	auto hControl = reinterpret_cast<HWND>(lParam);
	if (hControl == hCheck)
	{
		return (LRESULT)((HBRUSH)hBrush);
	}
	else if (hControl ==hContent ) {
		// if edit control is in dialog procedure change LRESULT to INT_PTR
		//SetBkMode(hdc, TRANSPARENT);
		SetBkColor(hdc,RGB(255, 255, 255));
		SetTextColor(hdc, RGB(0, 0, 0));
		return (LRESULT)((HRESULT)hBrushContent);
	}
	return ::DefWindowProc(m_hWnd, nMsg, wParam, lParam);
}

LRESULT NeonWindow::OnContentClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled)
{
	if (Securehashsum::Instance().IsEmpty()) {
		content.clear();
		::SetWindowTextW(hContent, content.data());
		::InvalidateRect(hContent,nullptr,false);
	}
	return S_OK;
}

LRESULT NeonWindow::Filesumsave(const std::wstring &file) {
	auto i = ComboBox_GetCurSel(hCombo);
	if (!ComboHashValue(i, fse)) {
		return S_OK;
	}
	UpdateTitle(PathFindFileNameW(file.data()));
	fse.file = file;
	fse.callback = [this](std::int32_t state, std::uint32_t progress_, std::wstring data) {
		return this->FilesumInvoke(state, progress_, data);
	};
	Securehashsum::Instance().Push(fse);
	return S_OK;
}

LRESULT NeonWindow::OnOpenFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled)
{
	std::wstring filename;
	if (!KismetDiscoverWindow(m_hWnd, filename, L"Open File")) {
		return S_OK;
	}
	return Filesumsave(filename);
}

LRESULT NeonWindow::OnDropfiles(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	HDROP hDrop = (HDROP)wParam;
	WCHAR file[32267];
	UINT nfilecounts = DragQueryFileW(hDrop, 0, file, sizeof(file));
	DragFinish(hDrop);
	if (nfilecounts ==0)return S_OK;
	return Filesumsave(file);
}
