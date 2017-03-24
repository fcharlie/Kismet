#ifndef SECURE_HASH_H
#define SECURE_HASH_H
#pragma once
#include <string>
#include <queue>
#include <unordered_map>
#include <functional>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <Windows.h>

enum FilesumAlm {
	kFilesumMD5,
	kFilesumSHA1,/// sha1 collision detection
	kFilesumSHA2,
	kFilesumSHA3,
	kFilesumCRC32,
};

enum FilesumState {
	kFilesumNone,
	kFilesumProgress,
	kFilesumCompleted,
	kFilesumCollCompleted,/// check result DC
	kFilesumBroken
};

struct FileSumElement {
	std::wstring file;/// file path
	std::uint32_t alm;/// 
	std::uint32_t width;///224 256 384 512
	std::function<bool(std::int32_t state,std::uint32_t progress,std::wstring data)> callback;/// if false , skip
};
typedef std::shared_ptr<FileSumElement> FileSumElementPtr;

class Securehashsum {
public:
	Securehashsum(const Securehashsum &) = delete;
	Securehashsum &operator=(const Securehashsum &) = delete;
	static Securehashsum &Instance() {
		static Securehashsum instance_;
		return instance_;
	}
	///
	bool InitializeSecureTask();
	bool Push(FileSumElement &filesum);
	FileSumElementPtr Pop();
private:
	Securehashsum() = default;
	std::queue<FileSumElementPtr> filesums;
	std::mutex mtx;
	std::condition_variable condv;
	DWORD tid{ 0 };
	HANDLE hThread{ nullptr };
};

class Securehash {
public:
private:
};

#endif