#include "stdafx.h"
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
#include <commdlg.h>
#include <ppltasks.h>
#include "Kismet.h"
#include "NeonWindow.h"
#include "MessageWindow.h"
#include "Hashsum.h"



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
	:pFactory(nullptr),
	AppPageBackgroundThemeBrush(nullptr),
	AppPageTextBrush(nullptr),
	pHwndRenderTarget(nullptr),
	pWriteFactory(nullptr),
	pLabelWriteTextFormat(nullptr)
{
}

NeonWindow::~NeonWindow()
{
	SafeRelease(&pLabelWriteTextFormat);
	SafeRelease(&pWriteFactory);
	SafeRelease(&AppPageBackgroundThemeBrush);
	SafeRelease(&AppPageTextBrush);
	SafeRelease(&pHwndRenderTarget);
	SafeRelease(&pFactory);
}
#define WS_NORESIZEWINDOW (WS_OVERLAPPED | WS_CAPTION |WS_SYSMENU | \
 WS_CLIPCHILDREN | WS_MINIMIZEBOX )

LRESULT NeonWindow::InitializeWindow()
{
	/// create D2D1
	if (Initialize() != S_OK) {
		::MessageBoxW(nullptr, L"Initialize() failed", L"Fatal error", MB_OK | MB_ICONSTOP);
		std::terminate();
		return S_FALSE;
	}
	HRESULT  hr = E_FAIL;
	RECT layout = { CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT+ 700, CW_USEDEFAULT+ 370 };
	width = 700;
	height = 370;
	areaheight = 250;
	Create(nullptr, layout, ns.title.data(),
		WS_NORESIZEWINDOW,
		WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
	return S_OK;
}

/////
HRESULT NeonWindow::CreateDeviceIndependentResources() {
	HRESULT hr = S_OK;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
	if (SUCCEEDED(hr)) {
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&pWriteFactory));
		if (SUCCEEDED(hr)) {
			hr = pWriteFactory->CreateTextFormat(
				ns.font.c_str(),
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				16.0f,
				L"zh-CN",
				&pLabelWriteTextFormat);
		}

	}
	return hr;
}
HRESULT NeonWindow::Initialize() {
	auto hr = CreateDeviceIndependentResources();
	//FLOAT dpiX, dpiY;
	if (hr == S_OK) {
		pFactory->ReloadSystemMetrics();
		pFactory->GetDesktopDpi(&dpiX, &dpiY);
	}
	return hr;
}
HRESULT NeonWindow::CreateDeviceResources() {
	HRESULT hr = S_OK;

	if (!pHwndRenderTarget) {
		RECT rc;
		::GetClientRect(m_hWnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU(
			rc.right - rc.left,
			rc.bottom - rc.top
		);
		hr = pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hWnd, size),
			&pHwndRenderTarget
		);
		if (SUCCEEDED(hr)) {
			////
			hr = pHwndRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF((UINT32)ns.panelcolor),
				&AppPageBackgroundThemeBrush
			);
		}
		if (SUCCEEDED(hr)) {
			hr = pHwndRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(ns.labelcolor),
				&AppPageTextBrush
			);
		}
	}
	return hr;
}
void NeonWindow::DiscardDeviceResources() {
	SafeRelease(&AppPageBackgroundThemeBrush);
	SafeRelease(&AppPageTextBrush);
}
HRESULT NeonWindow::OnRender() {
	auto hr = CreateDeviceResources();
	if (SUCCEEDED(hr)) {
#pragma warning(disable:4244)
#pragma warning(disable:4267)
		if (SUCCEEDED(hr)) {
			auto size = pHwndRenderTarget->GetSize();
			pHwndRenderTarget->BeginDraw();
			pHwndRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			pHwndRenderTarget->Clear(D2D1::ColorF(ns.contentcolor));
			pHwndRenderTarget->FillRectangle(
				D2D1::RectF(0,areaheight,size.width,size.height),
				AppPageBackgroundThemeBrush);
			wchar_t uppercase[] = L"Uppercase";
			pHwndRenderTarget->DrawTextW(uppercase, 9,
				pLabelWriteTextFormat,
				D2D1::RectF(40, areaheight+28.0f,200.0f, areaheight+50.0f), AppPageTextBrush,
				D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
				DWRITE_MEASURING_MODE_NATURAL);
			if (filetext.size()) {
				const wchar_t Name[] = L"Name:";
				pHwndRenderTarget->DrawTextW(Name, 5,
					pLabelWriteTextFormat,
					D2D1::RectF(20, 5.0f, keywidth, lineheight), AppPageTextBrush,
					D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
					DWRITE_MEASURING_MODE_NATURAL);
					pHwndRenderTarget->DrawTextW(filetext.data(), filetext.size(),
						pLabelWriteTextFormat,
						D2D1::RectF(keywidth, 5.0f, size.width-20, lineheight), AppPageTextBrush,
						D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
						DWRITE_MEASURING_MODE_NATURAL);
			}
			if (sizetext.size()) {
				const wchar_t Size[] = L"Size:";
				pHwndRenderTarget->DrawTextW(Size, 5,
					pLabelWriteTextFormat,
					D2D1::RectF(20, lineheight+5.0f, keywidth, lineheight*2), AppPageTextBrush,
					D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
					DWRITE_MEASURING_MODE_NATURAL);
				pHwndRenderTarget->DrawTextW(sizetext.data(), sizetext.size(),
					pLabelWriteTextFormat,
					D2D1::RectF(keywidth, lineheight+5.0f, size.width - 20, lineheight*2), AppPageTextBrush,
					D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
					DWRITE_MEASURING_MODE_NATURAL);
			}
			if (hash.size()) {
				const wchar_t Size[] = L"Hash:";
				pHwndRenderTarget->DrawTextW(Size, 5,
					pLabelWriteTextFormat,
					D2D1::RectF(20, lineheight*2+5.0f, keywidth, lineheight * 3), AppPageTextBrush,
					D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
					DWRITE_MEASURING_MODE_NATURAL);
				pHwndRenderTarget->DrawTextW(hash.data(), hash.size(),
					pLabelWriteTextFormat,
					D2D1::RectF(keywidth, lineheight*2+5.0f, size.width - 20, lineheight * 5), AppPageTextBrush,
					D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
					DWRITE_MEASURING_MODE_NATURAL);
			}
			//// write progress 100 %
			if (progress == 100) {
				wchar_t text[] = L"\xD83D\xDCAF";
				pHwndRenderTarget->DrawTextW(
					text, 2, pLabelWriteTextFormat,
					D2D1::RectF(160.0f, areaheight + 28.0f, 250.0f, areaheight + 50.5f),
					AppPageTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
					DWRITE_MEASURING_MODE_NATURAL);
			}
			else if (progress > 0 && progress < 100) {
				auto progressText = std::to_wstring(progress) + L"%";
				pHwndRenderTarget->DrawTextW(
					progressText.c_str(), progressText.size(), pLabelWriteTextFormat,
					D2D1::RectF(160.0f, areaheight + 28.0f, 250.0f, areaheight + 50.5f),
					AppPageTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
					DWRITE_MEASURING_MODE_NATURAL);
			}
			else if (showerror) {
				wchar_t text[] = L"\xD83D\xDE1F";
				pHwndRenderTarget->DrawTextW(
					text, 2, pLabelWriteTextFormat,
					D2D1::RectF(160.0f, areaheight + 28.0f, 250.0f, areaheight + 50.5f),
					AppPageTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
					DWRITE_MEASURING_MODE_NATURAL);
			}
			else {
				wchar_t text[] = L"\x2764Kismet";
				pHwndRenderTarget->DrawTextW(
					text, 7, pLabelWriteTextFormat,
					D2D1::RectF(160.0f, areaheight + 28.0f, 250.0f, areaheight + 50.5f),
					AppPageTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
					DWRITE_MEASURING_MODE_NATURAL);
			}
			hr = pHwndRenderTarget->EndDraw();
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
	if (pHwndRenderTarget) {
		pHwndRenderTarget->Resize(D2D1::SizeU(width, height));
	}
}

void NeonWindow::UpdateTitle(const std::wstring & title_)
{
	std::wstring xtitle = ns.title;
	if (!title_.empty()) {
		xtitle.append(L" - ").append(title_);
	}
	SetWindowTextW(xtitle.data());
}

bool NeonWindow::UpdateTheme()
{
	if (hBrush) {
		DeleteObject(hBrush);
	}
	hBrush = CreateSolidBrush(calcLuminance(ns.panelcolor));
	SafeRelease(&AppPageBackgroundThemeBrush);
	auto hr = pHwndRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF((UINT32)ns.panelcolor),
		&AppPageBackgroundThemeBrush
	);
	::InvalidateRect(hCheck, nullptr, TRUE);
	InvalidateRect(nullptr, TRUE);
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
	::SendMessage(hWnd, CB_SETCURSEL, 0, 0);
	return true;
}

inline bool HashsumAlgmCheck(int i, FilesumAlgw &aw) {
	switch (i) {
	case 0:
		aw.alm = kFilesumMD5;
		aw.name = L"MD5";
		break;
	case 1:
		aw.alm = kFilesumSHA1;
		aw.name = L"SHA1";
		break;
	case 2:
		aw.alm = kFilesumSHA1DC;
		aw.name = L"SHA1-DC";
		break;
	case 3:
		aw.alm = kFilesumSHA2;
		aw.width = 224;
		aw.name = L"SHA224";
		break;
	case 4:
		aw.alm = kFilesumSHA2;
		aw.width = 256;
		aw.name = L"SHA256";
		break;
	case 5:
		aw.alm = kFilesumSHA2;
		aw.width = 384;
		aw.name = L"SHA384";
		break;
	case 6:
		aw.alm = kFilesumSHA2;
		aw.name = L"SHA512";
		aw.width = 512;
		break;
	case 7:
		aw.alm = kFilesumSHA3;
		aw.width = 224;
		aw.name = L"SHA3-224";
		break;
	case 8:
		aw.alm = kFilesumSHA3;
		aw.width = 256;
		aw.name = L"SHA3-256";
		break;
	case 9:
		aw.alm = kFilesumSHA3;
		aw.width = 384;
		aw.name = L"SHA3-384";
		break;
	case 10:
		aw.alm = kFilesumSHA3;
		aw.width = 512;
		aw.name = L"SHA3-512";
		break;
	default:
		return false;
	}
	return true;
}


//// Window  style-ex
#define KWS_WINDOWEX WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY
//// Edit style
#define KWS_EDIT  WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_VISIBLE |WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL
/// push button style
#define KWS_BUTTON BS_PUSHBUTTON |BS_FLAT|BS_TEXT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE
/// checkbox style
#define KWS_CHECKBOX BS_FLAT|BS_TEXT  | BS_CHECKBOX | BS_AUTOCHECKBOX | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE
/// combobox style
#define KWS_COMBOBOX WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP | \
CBS_DROPDOWNLIST  | CBS_HASSTRINGS 
#define DEFAULT_PADDING96    20
LRESULT NeonWindow::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandle)
{
	HICON hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDI_KISMET));
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
	auto dpi = GetDpiForWindow(m_hWnd);
	
	logFont.lfHeight = MulDiv(DEFAULT_PADDING96, dpi, 96);
	logFont.lfWeight = FW_NORMAL;
	wcscpy_s(logFont.lfFaceName, L"Segoe UI");
	
	hFont = CreateFontIndirectW(&logFont);
	auto LambdaCreateWindow = [&](LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle,
		int X, int Y, int nWidth, int nHeight, HMENU hMenu)->HWND {
		auto hw = CreateWindowExW(KWS_WINDOWEX, lpClassName, lpWindowName,
			dwStyle, X, Y, nWidth, nHeight, m_hWnd, hMenu, HINST_THISCOMPONENT, nullptr);
		if (hw) {
			::SendMessageW(hw, WM_SETFONT, (WPARAM)hFont, lParam);
		}
		return hw;
	};
	auto LambdaCreateWindowEdge = [&](LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle,
		int X, int Y, int nWidth, int nHeight, HMENU hMenu)->HWND {
		auto hw = CreateWindowExW(KWS_WINDOWEX | WS_EX_CLIENTEDGE, lpClassName, lpWindowName,
			dwStyle, X, Y, nWidth, nHeight, m_hWnd, hMenu, HINST_THISCOMPONENT, nullptr);
		if (hw) {
			::SendMessageW(hw, WM_SETFONT, (WPARAM)hFont, lParam);
		}
		return hw;
	};
	RECT rect;
	GetWindowRect(&rect);

	hCheck = LambdaCreateWindow(WC_BUTTONW, 
		L"", 
		KWS_CHECKBOX, 
		20, areaheight+30, 20, 20, nullptr);

	hCombo = LambdaCreateWindow(WC_COMBOBOXW, L"",
		KWS_COMBOBOX,
		width- 420, areaheight+25, 120, 27, nullptr);

	hOpenButton = LambdaCreateWindow(WC_BUTTONW,
		L"Clear",
		KWS_BUTTON,
		width - 290, areaheight+25, 120, 27,
		HMENU(IDC_CLEAR_BUTTON));

	hOpenButton = LambdaCreateWindow(WC_BUTTONW, 
		L"Open", 
		KWS_BUTTON,
		width-160, areaheight+25, 120, 27, 
		HMENU(IDC_FILEOPEN_BUTTON));

	InitializeComboHash(hCombo);
	hBrush = CreateSolidBrush(calcLuminance(ns.panelcolor));
	HMENU hSystemMenu = ::GetSystemMenu(m_hWnd, FALSE);
	InsertMenuW(hSystemMenu, SC_CLOSE, MF_ENABLED, IDM_CHANGE_THEME, L"Change Panel Color");
	InsertMenuW(hSystemMenu, SC_CLOSE, MF_ENABLED, IDM_KISMET_INFO, L"About Kismet Immersive\tAlt+F1");
	return S_OK;
}

LRESULT NeonWindow::OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandle)
{
	if (hBrush) {
		DeleteObject(hBrush);
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

bool IsKeyPressed(UINT nVirtKey)
{
	return GetKeyState(nVirtKey) < 0 ? true : false;
}

LRESULT NeonWindow::OnKeydown(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	bool fCtrlDown = IsKeyPressed(VK_CONTROL);
	switch (wParam) {
	case 'c':
	case 'C':
		CopyToClipboard(hash);
		MessageBeep(MB_OK);
		return S_OK;
	default:
		return 0;
	}

	return S_OK;
}

LRESULT NeonWindow::OnColorStatic(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandle)
{
	HDC hdc = (HDC)wParam;
	auto hControl = reinterpret_cast<HWND>(lParam);
	if (hControl == hCheck)
	{
		SetBkMode(hdc, TRANSPARENT);
		//SetBkColor(hdc, RGB(255, 255, 255));
		SetTextColor(hdc, calcLuminance(ns.labelcolor));
		return (LRESULT)((HBRUSH)hBrush);
	}
	return ::DefWindowProc(m_hWnd, nMsg, wParam, lParam);
}

LRESULT NeonWindow::OnColorButton(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandle)
{
	HDC hdc = (HDC)wParam;
	SetTextColor(hdc, RGB(0, 0, 0));
	SetBkMode(hdc,TRANSPARENT);
	return (LRESULT)((HRESULT)hBrush);
}


LRESULT NeonWindow::OnRButtonUp(UINT, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	POINT pt;
	GetCursorPos(&pt);
	auto ptc = pt;
	ScreenToClient(&pt);
	if (pt.y >= (LONG)areaheight) {
		//// NOT Area
		return S_OK;
	}
	HMENU hMenu = ::LoadMenuW(GetModuleHandle(nullptr), MAKEINTRESOURCEW(IDM_KISMET_CONTEXT));
	HMENU hPopu = GetSubMenu(hMenu, 0);
	if (hash.empty()) {
		EnableMenuItem(hPopu, IDM_CONTEXT_COPY, MF_DISABLED);
	}
	else {
		EnableMenuItem(hPopu, IDM_CONTEXT_COPY, MF_ENABLED);
	}

	::TrackPopupMenuEx(hPopu, TPM_RIGHTBUTTON, ptc.x, ptc.y, m_hWnd, nullptr);
	::DestroyMenu(hPopu);
	return S_OK;
}

LRESULT NeonWindow::OnContentClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled)
{
	if (!locked) {
		filetext.clear();
		sizetext.clear();
		hash.clear();
		showerror = false;
		progress = 0;
		UpdateTitle(L"");
		InvalidateRect(nullptr, false);
	}
	return S_OK;
}


LRESULT NeonWindow::Filesum(const std::wstring & file)
{
	if (file.empty()) {
		return S_FALSE;
	}
	if (locked) {
		return S_FALSE;
	}
	FilesumAlgw aw;
	if (!HashsumAlgmCheck(ComboBox_GetCurSel(hCombo), aw)) {
		return false;
	}
	filetext.clear();
	hash.clear();
	sizetext.clear();
	//InvalidateRect(nullptr);
	std::wstring title;
	title.append(L"(").append(aw.name).append(L") ").append(PathFindFileNameW(file.data()));
	UpdateTitle(title);
	showerror = false;
	Concurrency::create_task([this, file,aw]()->bool {
		std::shared_ptr<Hashsum> sum(CreateHashsum(file, aw.alm, aw.width));
		if (!sum) {
			return false;
		}
		AllocSingle as;
		BYTE *buffer = as.Alloc<BYTE>(AllocSingle::kInternalBufferSize);
		if (as.size() == 0 || buffer == nullptr) {
			return false;
		}
		auto hFile = CreateFileW(file.data(),
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			nullptr);
		if (hFile == INVALID_HANDLE_VALUE) {
			return false;
		}
		LARGE_INTEGER li;
		GetFileSizeEx(hFile, &li);
		if (file.size() > 64) {
			filetext.assign(PathFindFileNameW(file.data()));
			if (filetext.size() > 64)
			{
				filetext.resize(64);
				filetext.append(L"...");
			}
		}
		else {
			filetext = file;
		}
		sizetext.assign(std::to_wstring(li.QuadPart));
		DWORD dwRead;
		int64_t cmsize = 0;
		uint32_t pg = 0;
		for (;;) {
			if (!ReadFile(hFile, buffer, AllocSingle::kInternalBufferSize, &dwRead, nullptr)) {
				break;
			}
			sum->Update(buffer, dwRead);
			cmsize += dwRead;
			auto N = (uint32_t)(cmsize * 100 / li.QuadPart);
			progress = (uint32_t)N;
			/// when number is modify, Flush Window
			if (pg != N) {
				pg = (uint32_t)N;
				InvalidateRect(nullptr);
			}
			if (dwRead<AllocSingle::kInternalBufferSize)
				break;
		}
		CloseHandle(hFile);
		hash.clear();
		bool ucase = (Button_GetCheck(hCheck) == BST_CHECKED);
		sum->Final(ucase, hash);
		return true;
	}).then([this](bool result) {
		if (!result) {
			showerror = true;
		}
		//InvalidateRect(nullptr);
		locked = false;
	});
	return S_OK;
}

LRESULT NeonWindow::OnDropfiles(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	HDROP hDrop = (HDROP)wParam;
	WCHAR file[32267];
	UINT nfilecounts = DragQueryFileW(hDrop, 0, file, sizeof(file));
	DragFinish(hDrop);
	if (nfilecounts == 0)return S_OK;
	return Filesum(file);
}



LRESULT NeonWindow::OnOpenFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled)
{
	std::wstring filename;
	if (!KismetDiscoverWindow(m_hWnd, filename, L"Open File")) {
		return S_OK;
	}
	return Filesum(filename);
}

static COLORREF CustColors[] =
{
	RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),
	RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),
	RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),
	RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),
};


LRESULT NeonWindow::OnTheme(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled)
{
	auto color = calcLuminance(ns.panelcolor);
	CHOOSECOLOR co;
	ZeroMemory(&co, sizeof(co));
	co.lStructSize = sizeof(CHOOSECOLOR);
	co.hwndOwner = m_hWnd;
	co.lpCustColors = (LPDWORD)CustColors;
	co.rgbResult = calcLuminance(ns.panelcolor);
	co.lCustData = 0;
	co.lpTemplateName = nullptr;
	co.lpfnHook = nullptr;
	co.Flags = CC_FULLOPEN | CC_RGBINIT;
	if (ChooseColor(&co))
	{
		auto r = GetRValue(co.rgbResult);
		auto g = GetGValue(co.rgbResult);
		auto b = GetBValue(co.rgbResult);
		ns.panelcolor = (r << 16) + (g << 8) + b;
		UpdateTheme();
		ApplyWindowSettings(ns);
	}
	return S_OK;
}

LRESULT NeonWindow::OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled)
{
	MessageWindowEx(
		m_hWnd,
		L"About Kismet utilities",
		L"Prerelease: 1.0.0.0\nCopyright \xA9 2017, Force Charlie. All Rights Reserved.",
		L"For more information about this tool.\nVisit: <a href=\"http://forcemz.net/\">forcemz.net</a>",
		kAboutWindow);
	return S_OK;
}

bool NeonWindow::CopyToClipboard(const std::wstring &text) {
	if (text.empty())
		return false;
	if (!OpenClipboard()) {
		return false;
	}
	if (!EmptyClipboard()) {
		CloseClipboard();
		return false;
	}
	HGLOBAL hgl = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(wchar_t));
	if (hgl==nullptr) {
		CloseClipboard();
		return false;
	}
	LPWSTR ptr = (LPWSTR)GlobalLock(hgl);
	memcpy(ptr, text.data(), (text.size() + 1) * sizeof(wchar_t));
	SetClipboardData(CF_UNICODETEXT, hgl);
	CloseClipboard();
	return true;
}

LRESULT NeonWindow::OnCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled)
{
	CopyToClipboard(hash);
	return S_OK;
}

