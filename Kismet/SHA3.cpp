#include "stdafx.h"
#include "Securehash.h"
#include "../rhash/sha3.h"


bool SHA3Sum(const FileSumElement &fse) {
	char to_hex[] = "0123456789abcdef";
	sha3_ctx ctx;
	switch (fse.width)
	{
	case 224:
		rhash_sha3_224_init(&ctx);
		break;
	case 256:
		rhash_sha3_256_init(&ctx);
		break;
	case 384:
		rhash_sha3_384_init(&ctx);
		break;
	case 512:
		rhash_sha3_512_init(&ctx);
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
	BYTE data[sha3_512_hash_size];
	DWORD dwRead;
	int64_t cmsize = 0;
	for (;;) {
		if (!ReadFile(hFile, buffer, sizeof(buffer), &dwRead, nullptr)) {
			break;
		}
		rhash_sha3_update(&ctx, buffer, dwRead);
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
	rhash_sha3_final(&ctx, data);
	std::wstring hex;
	for (uint32_t i = 0; i < fse.width/8; i++) {
		unsigned int val = data[i];
		hex.push_back(to_hex[val >> 4]);
		hex.push_back(to_hex[val & 0xf]);
	}
	fse.callback(kFilesumCompleted, 100, hex);
	return true;
}