#include "stdafx.h"
#include "Securehash.h"
#include "../rhash/md5.h"

bool MD5Sum(const FilesumEm &fse) {
	char to_hex[] = "0123456789abcdef";
	md5_ctx ctx;
	rhash_md5_init(&ctx);
	BYTE buffer[65536];
	BYTE data[md5_hash_size];
	DWORD dwRead;
	int64_t cmsize = 0;
	auto hFile = CreateFileW(fse.file.data(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);
	if (hFile == INVALID_HANDLE_VALUE) {
		fse.callback(kFilesumBroken, 0, L"Invalid handle value");
		return false;
	}
	LARGE_INTEGER li;
	GetFileSizeEx(hFile, &li);
	auto Ptr = reinterpret_cast<wchar_t *>(buffer);
	_snwprintf_s(Ptr, sizeof(buffer) / 2, sizeof(buffer) / 2,
		L"File:\t%s\r\nSize:\t%lld\r\nMD5:\t", fse.file.data(), li.QuadPart);
	fse.callback(kFilesumMessage, 0, Ptr);
	for (;;) {
		if (!ReadFile(hFile, buffer, sizeof(buffer), &dwRead, nullptr)) {
			break;
		}
		rhash_md5_update(&ctx, buffer, dwRead);
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
	rhash_md5_final(&ctx, data);
	std::wstring hex;
	for (uint32_t i = 0; i < sizeof(data); i++) {
		unsigned int val = data[i];
		hex.push_back(to_hex[val >> 4]);
		hex.push_back(to_hex[val & 0xf]);
	}
	fse.callback(kFilesumCompleted, 100, hex);
	return true;
}

