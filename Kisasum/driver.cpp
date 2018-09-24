#include "console.hpp"
#include "kisasum.h"
#include <cwctype>
#include <memory>
#include <unordered_map>

class AllocSingle {
public:
  enum {
    kInternalBufferSize = 1024 * 1024,
  };
  AllocSingle(const AllocSingle &) = delete;
  AllocSingle &operator=(const AllocSingle &) = delete;
  AllocSingle() = default;
  ~AllocSingle() {
    if (data) {
      HeapFree(GetProcessHeap(), 0, data);
    }
  }
  template <typename T>

  T *Alloc(size_t N) {
    if (data)
      return nullptr;
    data = HeapAlloc(GetProcessHeap(), 0, N);
    if (data != nullptr) {
      size_ = N;
    }
    return reinterpret_cast<T *>(data);
  }
  size_t size() const { return size_; }

private:
  void *data{nullptr};
  size_t size_{0};
};

std::wstring PathFilename(const wchar_t *lpszPath) {
  std::wstring name;
  auto lastSlash = lpszPath;
  while (lpszPath && *lpszPath) {
    if ((*lpszPath == '\\' || *lpszPath == '/' || *lpszPath == ':') &&
        lpszPath[1] && lpszPath[1] != '\\' && lpszPath[1] != '/')
      lastSlash = lpszPath + 1;
    lpszPath++;
  }
  name.assign(lastSlash);
  if (name.back() == '\\' || name.back() == '/') {
    name.pop_back();
  }
  return name;
}

inline std::wstring towlower(const std::wstring &raw) {
  std::wstring ws;
  for (auto c : raw) {
    ws.push_back(std::towlower(c));
  }
  return ws;
}

KisasumAlgorithm KisasumAlgorithmResolve(const std::wstring &alg, int &width) {
  auto la = towlower(alg);
  std::unordered_map<std::wstring, std::pair<KisasumAlgorithm, int>> algs = {
      {L"md5", {KisasumMD5, 0}},         {L"sha1", {KisasumSHA1, 0}},
      {L"sha1dc", {KisasumSHA1DC, 0}},   {L"sha224", {KisasumSHA2, 224}},
      {L"sha256", {KisasumSHA2, 256}},   {L"sha384", {KisasumSHA2, 384}},
      {L"sha512", {KisasumSHA2, 512}},   {L"sha3-224", {KisasumSHA3, 224}},
      {L"sha3-256", {KisasumSHA3, 256}}, {L"sha3-384", {KisasumSHA3, 384}},
      {L"sha3-512", {KisasumSHA3, 512}}};
  auto iter = algs.find(la);
  if (iter == algs.end()) {
    return KisasumNone;
  }
  width = iter->second.second;
  return iter->second.first;
}

bool KisasumCaclulateElem(bool out, KisasumAlgorithm alm, int width,
                          const std::wstring_view &file, KisasumElement &elm) {
  AllocSingle as;
  BYTE *buffer = as.Alloc<BYTE>(AllocSingle::kInternalBufferSize);
  if (as.size() == 0 || buffer == nullptr) {
    return false;
  }
  auto hFile = CreateFileW(file.data(), GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (hFile == INVALID_HANDLE_VALUE) {
    return false;
  }
  LARGE_INTEGER li;
  GetFileSizeEx(hFile, &li);
  elm.filename = PathFilename(file.data());
  DWORD dwRead;
  console::ProgressBar pb;
  pb.Initialize(li.QuadPart);
  std::shared_ptr<KisasumBase> sum(CreateKisasum(alm, width));
  for (;;) {
    if (!ReadFile(hFile, buffer, AllocSingle::kInternalBufferSize, &dwRead,
                  nullptr)) {
      break;
    }
    sum->Update(buffer, dwRead);
    if (out) {
      /// progress bar update.
      pb.Update(dwRead);
    }
    if (dwRead < AllocSingle::kInternalBufferSize)
      break;
  }
  sum->Final(true, elm.hash);
  CloseHandle(hFile);
  if (out) {
    pb.Refresh();
    console::PrintNone(L"\r%s    %s\n", elm.hash, elm.filename);
  }
  return true;
}

bool KisasumCalculate(const KisasumFilelist &list, KisasumResult &result) {
  int width = 0;
  auto alg = KisasumAlgorithmResolve(list.algorithm, width);
  if (alg == KisasumNone) {
    console::Print(console::fc::Red, L"Invalid Algorithm: %s", list.algorithm);
    return false;
  }
  if (list.format == KisasumText) {
    console::PrintNone(L"Kisasum calculate algorithm: %s\n", list.algorithm);
  } else {
    result.algorithm.assign(list.algorithm);
  }
  for (const auto &f : list.files) {
    KisasumElement elem;
    if (KisasumCaclulateElem(list.format == KisasumText, alg, width, f, elem)) {
      result.elems.push_back(std::move(elem));
    }
  }
  return true;
}

int wmain(int argc, wchar_t **argv) {
  KisasumFilelist filelist;
  if (ProcessArgv(argc, argv, filelist) != 0) {
    return 1;
  }
  if (filelist.files.empty()) {
    fprintf(stderr, "usage: kisasum [options] file file1...");
    return 1;
  }
  KisasumResult result;
  if (!KisasumCalculate(filelist, result)) {
    return 1;
  }
  if (filelist.format == KisasumJSON) {
    return KisasumPrintJSON(result);
  }
  if (filelist.format == KisasumXML) {
    return KisasumPrintXML(result);
  }
  return 0;
}