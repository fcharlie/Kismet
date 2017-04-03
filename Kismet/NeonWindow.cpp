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
	if (ns.title.empty()) {
		ns.title.assign(L"Kismet Immersive");
	}
	width = 700;
	height = 370;
	areaheigh = 250;
	Create(nullptr, layout, ns.title.data(),
		WS_NORESIZEWINDOW,
		WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
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
				16.0f,
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
				D2D1::ColorF((UINT32)ns.panelcolor),
				&m_pBackgroundBrush
			);
		}
		if (SUCCEEDED(hr)) {
			hr = m_pHwndRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(ns.labelcolor),
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
			m_pHwndRenderTarget->Clear(D2D1::ColorF(ns.contentcolor));
			m_pHwndRenderTarget->FillRectangle(
				D2D1::RectF(0,areaheigh,size.width,size.height),
				m_pBackgroundBrush);
			wchar_t uppercase[] = L"Uppercase";
			m_pHwndRenderTarget->DrawTextW(uppercase, 9,
				m_pWriteTextFormat,
				D2D1::RectF(40, areaheigh+28.0f,200.0f, areaheigh+50.0f), m_pNormalTextBrush,
				D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
				DWRITE_MEASURING_MODE_NATURAL);
			if (content.size()) {
				m_pHwndRenderTarget->DrawTextW(content.data(), content.size(),
					m_pWriteTextFormat,
					D2D1::RectF(20, 0.0f, size.width-20, size.height - 100.0f), m_pNormalTextBrush,
					D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
					DWRITE_MEASURING_MODE_NATURAL);
			}
			//// write progress 100 %
			if (progress == 100) {
				wchar_t text[] = L"\xD83D\xDCAF";
				m_pHwndRenderTarget->DrawTextW(
					text, 2, m_pWriteTextFormat,
					D2D1::RectF(160.0f, areaheigh + 28.0f, 250.0f, areaheigh + 50.5f),
					m_pNormalTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
					DWRITE_MEASURING_MODE_NATURAL);
			}
			else if (progress > 0 && progress < 100) {
				auto progressText = std::to_wstring(progress) + L"%";
				m_pHwndRenderTarget->DrawTextW(
					progressText.c_str(), progressText.size(), m_pWriteTextFormat,
					D2D1::RectF(160.0f, areaheigh + 28.0f, 250.0f, areaheigh + 50.5f),
					m_pNormalTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
					DWRITE_MEASURING_MODE_NATURAL);
			}
			else if (showerror) {
				wchar_t text[] = L"\x3DD8\x14DE";
				m_pHwndRenderTarget->DrawTextW(
					text, 2, m_pWriteTextFormat,
					D2D1::RectF(160.0f, areaheigh + 28.0f, 250.0f, areaheigh + 50.5f),
					m_pNormalTextBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
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
	SafeRelease(&m_pBackgroundBrush);
	auto hr = m_pHwndRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF((UINT32)ns.panelcolor),
		&m_pBackgroundBrush
	);
	::InvalidateRect(hCheck, nullptr, TRUE);
	InvalidateRect(nullptr, TRUE);
	return true;
}



bool NeonWindow::FilesumInvoke(std::int32_t state, std::uint32_t pg, std::wstring data)
{
	switch (state) {
	case kFilesumBroken:
		return false;
	case kFilesumMessage:
		content.assign(std::move(data));
		break;
	case kFilesumCompleted:
		content.append(data);
		hash.assign(data);
		break;
	case kFilesumProgress:
		progress = pg;
		InvalidateRect(nullptr, false);
		break;
	default:
		return false;
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
		aw.name = L"S1-DC";
		break;
	case 3:
		aw.alm = kFilesumSHA2;
		aw.width = 224;
		aw.name = L"S2-224";
		break;
	case 4:
		aw.alm = kFilesumSHA2;
		aw.width = 256;
		aw.name = L"S2-256";
		break;
	case 5:
		aw.alm = kFilesumSHA2;
		aw.width = 384;
		aw.name = L"S2-384";
		break;
	case 6:
		aw.alm = kFilesumSHA2;
		aw.name = L"S2-512";
		aw.width = 512;
		break;
	case 7:
		aw.alm = kFilesumSHA3;
		aw.width = 224;
		aw.name = L"S3-224";
		break;
	case 8:
		aw.alm = kFilesumSHA3;
		aw.width = 256;
		aw.name = L"S3-256";
		break;
	case 9:
		aw.alm = kFilesumSHA3;
		aw.width = 384;
		aw.name = L"S3-384";
		break;
	case 10:
		aw.alm = kFilesumSHA3;
		aw.width = 512;
		aw.name = L"S3-512";
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

LRESULT NeonWindow::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandle)
{
	auto hr = Initialize();
	if (hr != S_OK) {
		::MessageBoxW(nullptr, L"Initialize() failed", L"Fatal error", MB_OK | MB_ICONSTOP);
		std::terminate();
		return S_FALSE;
	}
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

	logFont.lfHeight = 20;
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
		20, areaheigh+30, 20, 20, nullptr);

	hCombo = LambdaCreateWindow(WC_COMBOBOXW, L"",
		KWS_COMBOBOX,
		width- 420, areaheigh+25, 120, 27, nullptr);

	hOpenButton = LambdaCreateWindow(WC_BUTTONW,
		L"Clear",
		KWS_BUTTON,
		width - 290, areaheigh+25, 120, 27,
		HMENU(IDC_CLEAR_BUTTON));

	hOpenButton = LambdaCreateWindow(WC_BUTTONW, 
		L"Open", 
		KWS_BUTTON,
		width-160, areaheigh+25, 120, 27, 
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
	if (pt.y >= (LONG)areaheigh) {
		//// NOT Area
		return S_OK;
	}
	HMENU hMenu = ::LoadMenuW(GetModuleHandle(nullptr), MAKEINTRESOURCEW(IDM_KISMET_CONTEXT));
	HMENU hPopu = GetSubMenu(hMenu, 0);
	if (content.empty()) {
		EnableMenuItem(hPopu, IDM_CONTEXT_COPYALL, MF_DISABLED);
	}
	else {
		EnableMenuItem(hPopu, IDM_CONTEXT_COPYALL, MF_ENABLED);
	}
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
		content.clear();
		hash.clear();
		progress = 0;
		UpdateTitle(L"");
		InvalidateRect(nullptr, false);
	}
	return S_OK;
}

LRESULT NeonWindow::Filesum(const std::wstring & file)
{
	if (locked) {
		return S_FALSE;
	}
	FilesumAlgw aw;
	bool ucase = (Button_GetCheck(hCheck) == BST_CHECKED);
	if (!HashsumAlgmCheck(ComboBox_GetCurSel(hCombo), aw)) {
		return false;
	}
	//// create task
	UpdateTitle(PathFindFileNameW(file.data()));
	showerror = false;
	Concurrency::create_task([this, file,aw,ucase]()->bool {
		std::shared_ptr<Hashsum> sum(CreateHashsum(file, aw.alm, aw.width));
		if (!sum) {
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
			FilesumInvoke(kFilesumBroken, 0, L"Invalid handle value");
			return false;
		}
		BYTE buffer[65536];
		LARGE_INTEGER li;
		GetFileSizeEx(hFile, &li);
		auto Ptr = reinterpret_cast<wchar_t *>(buffer);
		std::wstring filename;
		if (file.size() > 64) {
			filename.assign(PathFindFileNameW(file.data()));
			if (filename.size() > 64)
			{
				filename.resize(64);
				filename.append(L"...");
			}
		}
		else {
			filename = file;
		}
		_snwprintf_s(Ptr, sizeof(buffer) / 2, sizeof(buffer) / 2,
			L"File:\t%s\r\nSize:\t%lld\r\n%s:\t", filename.data(), li.QuadPart,aw.name.data());
		FilesumInvoke(kFilesumMessage, 0, Ptr);
		DWORD dwRead;
		int64_t cmsize = 0;
		for (;;) {
			if (!ReadFile(hFile, buffer, sizeof(buffer), &dwRead, nullptr)) {
				break;
			}
			sum->Update(buffer, dwRead);
			cmsize += dwRead;
			auto N = cmsize * 100 / li.QuadPart;
			if (!FilesumInvoke(kFilesumProgress, (uint32_t)N, L""))
			{
				CloseHandle(hFile);
				return false;
			}
			if (dwRead<sizeof(buffer))
				break;
		}
		CloseHandle(hFile);
		std::wstring hash;
		sum->Final(ucase, hash);
		FilesumInvoke(kFilesumCompleted, 100, hash);
		return true;
	}).then([this](bool result) {
		if (!result) {
			showerror = true;
		}
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

