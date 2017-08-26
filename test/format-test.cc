#include <string>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <Windows.h>

struct KisasumElement {
	std::wstring filename;
	std::wstring hash;
};

struct KisasumResult {
	std::vector<KisasumElement> elems;
	std::wstring algorithm;
};

size_t WriteFormatted(const wchar_t * data, size_t len)
{
	auto hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwWrite = 0;
    WriteConsoleW(hConsole, data, (DWORD)len, &dwWrite, nullptr);
    return dwWrite;
}

int KisasumPrintJSON(const KisasumResult &result) {
    std::wstring ws(L"{\n"); /// start
	ws.append(L"  \"algorithm\": \"")
		.append(result.algorithm)
		.append(L"\",\n  \"files\": \n  [");
	for (const auto &e : result.elems) {
		ws.append(L"\n    {\n      \"name\": \"")
			.append(e.filename)
			.append(L"\",\n      \"hash\": \"")
			.append(e.hash)
			.append(L"\"\n    },");
	}
	if (ws.back() == L',') {
		ws.pop_back();
	}
	ws.append(L"\n  ]\n}\n");
    WriteFormatted(ws.data(), ws.size());
	return 0;
}
int KisasumPrintXML(const KisasumResult &result) {
	std::wstring ws(LR"(<?xml version="1.0">)");
	ws.append(L"\n<root>\n  <algorithm>")
		.append(result.algorithm)
		.append(L"</algorithm>\n  <files>\n");
	for (const auto &e: result.elems) {
		ws.append(L"    <file>\n      <name>")
			.append(e.filename)
			.append(L"</name>\n      <hash>")
			.append(e.hash)
			.append(L"</hash>\n    </file>\n");
	}
	ws.append(L"  </files>\n</root>\n");
	WriteFormatted(ws.data(), ws.size());
	return 0;
}

int wmain(){
    KisasumResult result;
    result.algorithm=L"SHA256";
    KisasumElement e1,e2;
    e1.filename=L"powershell-6.0.0-beta.6-osx.10.12-x64.pkg";
    e1.hash=L"E69C22F2224707223F32607D08F73C0BEADC3650069C49FC11C8071D08843309";
    e2.filename=L"PowerShell-6.0.0-beta.6-win10-win2016-x64.msi";
    e2.hash=L"2F0F6F030F254590C63CD47CC3C4CD1952D002639D2F27F37E616CD2A5DDE84C";
    result.elems.push_back(e1);
    result.elems.push_back(e2);
    KisasumPrintXML(result);
    KisasumPrintJSON(result);
    return 0;
}