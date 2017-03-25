#include "stdafx.h"
#include "Securehash.h"

bool MD5Sum(const FilesumEm &fse);
bool SHA1Sum(const FilesumEm &fse);
bool SHA1DCSum(const FilesumEm &fse);
bool SHA2Sum(const FilesumEm &fse);
bool SHA3Sum(const FilesumEm &fse);

bool KitmetSecureCore(const FilesumEm &fse) {
	switch (fse.alm) {
	case kFilesumMD5:
		return MD5Sum(fse);
	case kFilesumSHA1:
		return SHA1Sum(fse);
	case kFilesumSHA1DC:
		return  SHA1DCSum(fse);
	case kFilesumSHA2:
		return SHA2Sum(fse);
	case kFilesumSHA3:
		return SHA3Sum(fse);
	case kFilesumCRC32:
		break;
	default:
		break;
	}
	return true;
}

DWORD WINAPI KismetSecureTask(PVOID data) {
	(void)data;
	/// get task always run
	for (;;) {
		auto ptr = Securehashsum::Instance().Pop();
		if (ptr) {
			KitmetSecureCore(*ptr);
		}
	}
	return 0;
}

bool Securehashsum::InitializeSecureTask() {
	hThread = CreateThread(nullptr, 0, KismetSecureTask, nullptr, 0, &tid);
	if (hThread == nullptr)
		return false;
	return true;
}

bool Securehashsum::Push(FilesumEm &filesum) {
	std::unique_lock<std::mutex> lck(mtx);
	FilesumemPtr ptr(new FilesumEm(filesum));
	filesums.push(std::move(ptr));
	condv.notify_one();
	return true;
}

FilesumemPtr Securehashsum::Pop() {
	std::unique_lock<std::mutex> lck(mtx);
	if (filesums.empty()) {
		/// mutex is unlock by wait
		condv.wait(lck);
	}
	auto s = filesums.front();
	filesums.pop();
	return s;
}