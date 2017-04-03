#include "stdafx.h"
#include "Hashsum.h"
#include "../Hashsum/md5.h"
#include "../Hashsum/sha1.h"
#include "../Hashsum/sha1detectcoll.h"
#include "../Hashsum/sha256.h"
#include "../Hashsum/sha512.h"
#include "../Hashsum/sha3.h"

static inline void BinaryToHex(const unsigned char *buf, size_t len, std::wstring &str) {
	char to_hex[] = "0123456789abcdef";
	for (uint32_t i = 0; i < len; i++) {
		unsigned int val = buf[i];
		str.push_back(to_hex[val >> 4]);
		str.push_back(to_hex[val & 0xf]);
	}
}

static inline void BinaryToHexUCase(const unsigned char *buf, size_t len, std::wstring &str) {
	char to_hex[] = "0123456789ABCDEF";
	for (uint32_t i = 0; i < len; i++) {
		unsigned int val = buf[i];
		str.push_back(to_hex[val >> 4]);
		str.push_back(to_hex[val & 0xf]);
	}
}
class MD5Hashsum :public Hashsum {
public:
	void Initialize(int width) {
		(void)width;
		rhash_md5_init(&ctx);
	}
	void Update(const unsigned char *buf, size_t len) {
		rhash_md5_update(&ctx, buf, len);
	}
	void Final(bool ucase, std::wstring &hash) {
		unsigned char buf[md5_hash_size];
		rhash_md5_final(&ctx, buf);
		if (ucase)
			BinaryToHexUCase(buf, md5_hash_size, hash);
		else
			BinaryToHex(buf, md5_hash_size, hash);
	}
private:
	md5_ctx ctx;
};

class SHA1Sum :public Hashsum {
public:
	void Initialize(int width) {
		(void)width;
		rhash_sha1_init(&ctx);
	}
	void Update(const unsigned char *buf, size_t len) {
		rhash_sha1_update(&ctx, buf, len);
	}
	void Final(bool ucase, std::wstring &hash) {
		unsigned char buf[sha1_hash_size];
		rhash_sha1_final(&ctx, buf);
		if (ucase)
			BinaryToHexUCase(buf, md5_hash_size, hash);
		else
			BinaryToHex(buf, md5_hash_size, hash);
	}
private:
	sha1_ctx ctx;
};

class SHADC1Sum :public Hashsum {
public:
	void Initialize(int width) {
		(void)width;
		SHA1DCInit(&ctx);
	}
	void Update(const unsigned char *buf, size_t len) {
		SHA1DCUpdate(&ctx, (const char*)buf, len);
	}
	void Final(bool ucase, std::wstring &hash) {
		unsigned char buf[sha1_hash_size];
		auto i = SHA1DCFinal(buf, &ctx);
		if (ucase)
			BinaryToHexUCase(buf, md5_hash_size, hash);
		else
			BinaryToHex(buf, md5_hash_size, hash);
		if (i != 0) {
			hash.append(L" *coll* ");
		}
	}
private:
	SHA1_CTX ctx;
};

class SHA256Sum :public Hashsum {
public:
	void Initialize(int w) {
		width = w;
		if (width == 224)
			rhash_sha224_init(&ctx);
		else
			rhash_sha256_init(&ctx);
	}
	void Update(const unsigned char *buf, size_t len) {
		rhash_sha256_update(&ctx, buf, len);
	}
	void Final(bool ucase, std::wstring &hash) {
		unsigned char buf[sha256_hash_size];
		rhash_sha256_final(&ctx, buf);
		if (ucase)
			BinaryToHexUCase(buf, width / 8, hash);
		else
			BinaryToHex(buf, width / 8, hash);
	}
private:
	sha256_ctx ctx;
	int width;
};

class SHA512Sum :public Hashsum {
public:
	void Initialize(int w) {
		width = w;
		if (width == 384)
			rhash_sha384_init(&ctx);
		else
			rhash_sha512_init(&ctx);
	}
	void Update(const unsigned char *buf, size_t len) {
		rhash_sha512_update(&ctx, buf, len);
	}
	void Final(bool ucase, std::wstring &hash) {
		unsigned char buf[sha512_hash_size];
		rhash_sha512_final(&ctx, buf);
		if (ucase)
			BinaryToHexUCase(buf, width / 8, hash);
		else
			BinaryToHex(buf, width / 8, hash);
	}
private:
	sha512_ctx ctx;
	int width;
};

class SHA3Sum :public Hashsum {
public:
	void Initialize(int w) {
		width = w;
		switch (width)
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
			break;
		}
	}
	void Update(const unsigned char *buf, size_t len) {
		rhash_sha3_update(&ctx, buf, len);
	}
	void Final(bool ucase, std::wstring &hash) {
		unsigned char buf[sha3_512_hash_size];
		rhash_sha3_final(&ctx, buf);
		if (ucase)
			BinaryToHexUCase(buf, width / 8, hash);
		else
			BinaryToHex(buf, width / 8, hash);
	}
private:
	sha3_ctx ctx;
	int width;
};

Hashsum * CreateHashsum(const std::wstring & file, int alg, int width)
{
	Hashsum *sum = nullptr;
	switch (alg) {
	case kFilesumMD5:
		sum = new MD5Hashsum();
		sum->Initialize(width);
		break;
	case kFilesumSHA1:
		sum = new SHA1Sum();
		sum->Initialize(width);
		break;
	case kFilesumSHA1DC:
		sum = new SHADC1Sum();
		sum->Initialize(width);
		break;
	case kFilesumSHA2:
		if (width <= 256) {
			sum = new SHA256Sum();
			sum->Initialize(width);
		}
		else {
			sum = new SHA512Sum();
			sum->Initialize(width);
		}
		break;
	case kFilesumSHA3:
		sum = new SHA3Sum();
		sum->Initialize(width);
		break;
	default:
		return nullptr;
	}
	return sum;
}
