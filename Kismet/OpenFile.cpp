#include "stdafx.h"
#include <string>
#include <Uxtheme.h>
#include <ShlObj.h>
#include <tchar.h>
#include <strsafe.h>
#pragma warning(disable:4091)
#include <Shlwapi.h>


typedef COMDLG_FILTERSPEC FilterSpec;

const FilterSpec filterSpec[] =
{
	{ L"All Files (*.*)", L"*.*" }
};

void ReportErrorMessage(LPCWSTR pszFunction, HRESULT hr)
{
	wchar_t szBuffer[65535] = { 0 };
	if (SUCCEEDED(StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), 
		L"Call: %s Failed w/hr 0x%08lx ,Please Checker Error Code!", pszFunction, hr))) {
		int nB = 0;
		TaskDialog(nullptr, GetModuleHandle(nullptr),
				   L"Failed information",
				   L"Call Function Failed:",
				   szBuffer,
				   TDCBF_OK_BUTTON,
				   TD_ERROR_ICON,
				   &nB);
	}
}
bool OpenFileWindow(
	HWND hParent,
	std::wstring &filename,
	const wchar_t *pszWindowTitle)
{
	HRESULT hr=S_OK;
	IFileOpenDialog *pWindow = nullptr;
	IShellItem *pItem = nullptr;
	PWSTR pwszFilePath = nullptr;
	if (CoCreateInstance(
		__uuidof(FileOpenDialog),
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pWindow)
		) != S_OK) {
		ReportErrorMessage(L"FileOpenWindowProvider", hr);
		return false;
	}
	hr = pWindow->SetFileTypes(sizeof(filterSpec) / sizeof(filterSpec[0]), filterSpec);
	hr = pWindow->SetFileTypeIndex(1);
	hr = pWindow->SetTitle(pszWindowTitle ? pszWindowTitle : L"Open File Provider");
	hr = pWindow->Show(hParent);
	if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
		// User cancelled.
		hr = S_FALSE;
		goto done;
	}
	if (FAILED(hr)) { goto done; }
	hr = pWindow->GetResult(&pItem);
	if (FAILED(hr))
		goto done;
	hr = pItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pwszFilePath);
	if (FAILED(hr)) { goto done; }
	filename.assign(pwszFilePath);
done:
	if (pwszFilePath) {
		CoTaskMemFree(pwszFilePath);
	}
	if (pItem) {
		pItem->Release();
	}
	if (pWindow) {
		pWindow->Release();
	}
	return hr==S_OK;
}
