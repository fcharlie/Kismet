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
#include <atomic>
//#include "Securehash.h"
#include "Kismet.h"



struct NeonSettings {
	/// color
	std::uint32_t panelcolor{ 0x00BFFF };
	std::uint32_t contentcolor{ 0xffffff };
	std::uint32_t textcolor{ 0x000000 };
	std::uint32_t labelcolor{ 0x000000 };
	std::wstring title;
};

bool ApplyWindowSettings(NeonSettings &ns);

bool KismetDiscoverWindow(
	HWND hParent,
	std::wstring &filename,
	const wchar_t *pszWindowTitle);


//// what's fuck HEX color and COLORREF color, red <-- --> blue
//// this is LE CPU, BE CPU don't call 
inline COLORREF calcLuminance(UINT32 cr) {
	int r = (cr & 0xff0000) >> 16;
	int g = (cr & 0xff00) >> 8;
	int b = (cr & 0xff);
	return RGB(r,g,b);
}


struct D2D1Checkbox {
	RECT Layout;
	std::wstring Text;
	bool IsChecked;
};

#define NEONWINDOWNAME L"Neon.UI.Window"

#ifndef SYSCOMMAND_ID_HANDLER
#define SYSCOMMAND_ID_HANDLER(id, func) \
	if(uMsg == WM_SYSCOMMAND && id == LOWORD(wParam)) \
					{ \
		bHandled = TRUE; \
		lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
					}
#endif

#define NEON_WINDOW_CLASSSTYLE WS_OVERLAPPED | WS_SYSMENU | \
WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS&~WS_MAXIMIZEBOX
typedef CWinTraits<NEON_WINDOW_CLASSSTYLE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE> CMetroWindowTraits;


///Awesome 
class NeonWindow :public CWindowImpl<NeonWindow, CWindow, CMetroWindowTraits> {
private:
	ID2D1Factory *pFactory;
	ID2D1HwndRenderTarget* pHwndRenderTarget;
	ID2D1SolidColorBrush* AppPageBackgroundThemeBrush;
	ID2D1SolidColorBrush* AppPageTextBrush;
	IDWriteFactory* pWriteFactory;
	//IDWriteTextFormat* pTitleWriteTextFormat;//Alignment left
	//IDWriteTextFormat* pButtonWriteTextFormat; /// Alignment center
	IDWriteTextFormat* pLabelWriteTextFormat; /// Alignment left
	HRESULT CreateDeviceIndependentResources();
	HRESULT Initialize();
	HRESULT CreateDeviceResources();
	void DiscardDeviceResources();
	HRESULT OnRender();
	D2D1_SIZE_U CalculateD2DWindowSize();
	void OnResize(
		UINT width,
		UINT height
	);
	/////////// self control handle
	LRESULT Filesum(const std::wstring &file);
	void UpdateTitle(const std::wstring &title_);
	bool CopyToClipboard(const std::wstring &text);
	bool UpdateTheme();
	HWND hCombo{ nullptr };
	HWND hOpenButton{ nullptr };
	HWND hClearButton{ nullptr };
	HWND hCheck{ nullptr };
	HBRUSH hBrush{ nullptr };
	NeonSettings ns;
	std::wstring content;
	std::wstring hash;
	std::atomic_uint32_t progress{ 0 };
	std::atomic_bool locked{ false };
	std::uint32_t height;
	std::uint32_t width;
	std::uint32_t areaheigh;
	bool showerror{ false };

public:
	NeonWindow(const NeonWindow &) = delete;
	NeonWindow &operator=(const NeonWindow &) = delete;
	NeonWindow();
	~NeonWindow();
	NeonSettings &Settings() { return ns; }
	LRESULT InitializeWindow();
	DECLARE_WND_CLASS(NEONWINDOWNAME)
	BEGIN_MSG_MAP(MetroWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnColorStatic)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeydown);
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_DISPLAYCHANGE, OnDisplayChange)
		MESSAGE_HANDLER(WM_DROPFILES, OnDropfiles)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
		COMMAND_ID_HANDLER(IDC_CLEAR_BUTTON, OnContentClear)
		COMMAND_ID_HANDLER(IDC_FILEOPEN_BUTTON, OnOpenFile)
		COMMAND_ID_HANDLER(IDM_CONTEXT_COPY,OnCopy)
		SYSCOMMAND_ID_HANDLER(IDM_CHANGE_THEME,OnTheme)
		SYSCOMMAND_ID_HANDLER(IDM_KISMET_INFO,OnAbout)
	END_MSG_MAP()
	LRESULT OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
	LRESULT OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
	LRESULT OnClose(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
	LRESULT OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
	LRESULT OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
	LRESULT OnDisplayChange(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnKeydown(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnDropfiles(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnColorStatic(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
	LRESULT OnColorButton(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandle);
	LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContentClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOpenFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnTheme(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	bool FilesumInvoke(std::int32_t state, std::uint32_t progress_, std::wstring data);
};

#endif