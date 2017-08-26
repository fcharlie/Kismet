#ifndef KISASUM_H
#define KISASUM_H
#pragma once
#include <Windows.h>
#include <cstdio>
#include <cstdlib>
#include <string_view>
#include <vector>

enum KisasumFormat {
	KisasumText,
	KisasumJSON,
	KisasumXML
};

enum KisasumAlgorithm {
	KisasumNone=-1,
	KisasumMD5,
	KisasumSHA1,
	KisasumSHA1DC,
	KisasumSHA2,
	KisasumSHA3
};

class KisasumBase {
public:
	virtual void Initialize(int width)=0;
	virtual void Update(const unsigned char *buf, size_t len) = 0;
	virtual void Final(bool ucase, std::wstring &hash) = 0;
};

KisasumBase * CreateKisasum(int alg, int width);

struct KisasumElement {
	std::wstring filename;
	std::wstring hash;
};

struct KisasumFilelist {
	std::vector<std::wstring_view> files;/// --> print line ...
	std::wstring algorithm;///// md5 sha1dc ...... 
	KisasumFormat format{ KisasumText };
};


struct KisasumResult {
	std::vector<KisasumElement> elems;
	std::wstring algorithm;
};

bool KisasumCalculate(const KisasumFilelist &list,KisasumResult &result);
int KisasumPrintJSON(const KisasumResult &result);
int KisasumPrintXML(const KisasumResult &result);
void KisasumUsage();
int ProcessArgv(int Argc, wchar_t **Argv, KisasumFilelist &filelist);

#endif
