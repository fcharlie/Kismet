#include "stdafx.h"
#include "Securehash.h"
#include "../rhash/sha256.h"
#include "../rhash/sha512.h"

bool SHA256Sum(const FileSumElement &fse) {
	char to_hex[] = "0123456789abcdef";
	sha256_ctx ctx;
	switch (fse.width)
	{
	case 224:
		rhash_sha224_init(&ctx);
		break;
	case 256:
		rhash_sha256_init(&ctx);
		break;
	default:
		return false;
	}
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
	BYTE buffer[65536];
	BYTE data[16];
	DWORD dwRead;
	int64_t cmsize = 0;
	for (;;) {
		if (!ReadFile(hFile, buffer, sizeof(buffer), &dwRead, nullptr)) {
			break;
		}
		rhash_sha256_update(&ctx, buffer, dwRead);
		cmsize += dwRead;
		auto N = cmsize * 100 / li.QuadPart;
		fse.callback(kFilesumProgress, (uint32_t)N, L"");
		if (dwRead<sizeof(buffer))
			break;
	}
	CloseHandle(hFile);
	rhash_sha256_final(&ctx, data);
	std::wstring hex;
	for (uint32_t i = 0; i < fse.width/8; i++) {
		unsigned int val = data[i];
		hex.push_back(to_hex[val >> 4]);
		hex.push_back(to_hex[val & 0xf]);
	}
	fse.callback(kFilesumCompleted, 100, hex);
	return true;
}

bool SHA512Sum(const FileSumElement &fse) {
	char to_hex[] = "0123456789abcdef";
	sha512_ctx ctx;
	switch (fse.width)
	{
	case 384:
		rhash_sha384_init(&ctx);
		break;
	case 256:
		rhash_sha512_init(&ctx);
		break;
	default:
		return false;
	}
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
	BYTE buffer[65536];
	BYTE data[16];
	DWORD dwRead;
	int64_t cmsize = 0;
	for (;;) {
		if (!ReadFile(hFile, buffer, sizeof(buffer), &dwRead, nullptr)) {
			break;
		}
		rhash_sha512_update(&ctx, buffer, dwRead);
		cmsize += dwRead;
		auto N = cmsize * 100 / li.QuadPart;
		fse.callback(kFilesumProgress, (uint32_t)N, L"");
		if (dwRead<sizeof(buffer))
			break;
	}
	CloseHandle(hFile);
	rhash_sha512_final(&ctx, data);
	std::wstring hex;
	for (uint32_t i = 0; i < fse.width / 8; i++) {
		unsigned int val = data[i];
		hex.push_back(to_hex[val >> 4]);
		hex.push_back(to_hex[val & 0xf]);
	}
	fse.callback(kFilesumCompleted, 100, hex);
	return true;
}

bool SHA2Sum(const FileSumElement &fse) {
	if (fse.width <= 256)
		return SHA256Sum(fse);
	return SHA512Sum(fse);
}