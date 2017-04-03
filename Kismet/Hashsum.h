#ifndef HASHSUM_H
#define HASHSUM_H
#pragma once
#include <string>
#include <ppltasks.h>

enum FilesumAlm {
	kFilesumMD5,
	kFilesumSHA1,
	kFilesumSHA1DC,/// sha1 collision detection
	kFilesumSHA2,
	kFilesumSHA3
};

enum FilesumState {
	kFilesumNone,
	kFilesumMessage,
	kFilesumProgress,
	kFilesumCompleted,
	kFilesumBroken
};

struct FilesumAlgw {
	int alm;
	int width;
	std::wstring name;
};

class Hashsum {
public:
	virtual void Initialize(int width) = 0;
	virtual void Update(const unsigned char *buf, size_t len) = 0;
	virtual void Final(bool ucase, std::wstring &hash) = 0;
};

Hashsum *CreateHashsum(const std::wstring &file, int alg, int width);

class Promise {
public:
	Promise(const Promise &) = delete;
	Promise&operator=(const Promise&) = delete;
	Promise() = default;
private:
};



#endif