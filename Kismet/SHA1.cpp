#include "stdafx.h"
#include "Securehash.h"
#include "../rhash/sha1detectcoll.h"

bool SHA1Sum(const FileSumElement &fse) {
	char to_hex[] = "0123456789abcdef";
	auto hFile = CreateFileW(fse.file.data(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);
	if (hFile == INVALID_HANDLE_VALUE) {
		fse.callback(kFilesumBroken, 0, L"Invalid handle value");
		return false;
	}
	LARGE_INTEGER li;
	GetFileSizeEx(hFile, &li);
	SHA1_CTX ctx;
	SHA1DCInit(&ctx);
	BYTE buffer[65536];
	BYTE data[20];
	DWORD dwRead;
	int64_t cmsize = 0;
	for (;;) {
		if (!ReadFile(hFile, buffer, sizeof(buffer), &dwRead, nullptr)) {
			break;
		}
		SHA1DCUpdate(&ctx, (const char*)buffer, dwRead);
		cmsize += dwRead;
		auto N = cmsize * 100 / li.QuadPart;
		if (!fse.callback(kFilesumProgress, (uint32_t)N, L""))
		{
			CloseHandle(hFile);
			return false;
		}
		if (dwRead<sizeof(buffer))
			break;
	}
	CloseHandle(hFile);
	bool detect = (SHA1DCFinal(data, &ctx) == 0);
	std::wstring hex;
	for (uint32_t i = 0; i < 20; i++) {
		unsigned int val = data[i];
		hex.push_back(to_hex[val >> 4]);
		hex.push_back(to_hex[val & 0xf]);
	}
	if(detect)
		fse.callback(kFilesumCollCompleted, 100, hex);
	else
		fse.callback(kFilesumCompleted, 100, hex);
	return true;
}