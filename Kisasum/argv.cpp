/////
#include "console.hpp"
#include "kisasum.h"

void KisasumUsage() {
  const char *ua = R"(OVERVIEW: kisasum 1.0
USAGE: kisasum [options] <input>
OPTIONS:
  -a, --algorithm  Hash Algorithm,support algorithm described below.
                   Algorithm Ignore case, default sha256
                   SHA1DC is SHA-1 collision.
  -f, --format     Return information about hash in a format described below.
  -h, -?, --help   Print usage and exit.
  -v, --version    Print version and exit.

Algorithm:
  MD5        SHA1       SHA1DC
  SHA224     SHA256     SHA384     SHA512
  SHA3-224   SHA3-256   SHA3-384   SHA3-512

Formats:
  text     format to text, support progress
  json     format to json
  xml      format to xml

)";
  fprintf(stderr, "%s", ua);
}

KisasumFormat KisasumFormatResolve(const wchar_t *fmt) {
  if (_wcsicmp(fmt, L"json") == 0) {
    return KisasumJSON;
  }
  if (_wcsicmp(fmt, L"xml") == 0) {
    return KisasumXML;
  }
  return KisasumText;
}

class ArgvImpl {
public:
  ArgvImpl(int Argc, wchar_t **Argv) : Argc_(Argc), Argv_(Argv) {
    ///
  }
  ArgvImpl(const ArgvImpl &) = delete;
  ArgvImpl &operator=(const ArgvImpl &) = delete;
  bool IsArgument(const wchar_t *candidate, const wchar_t *argname) {
    return (wcscmp(candidate, argname) == 0);
  }
  bool IsArgument(const wchar_t *candidate, const wchar_t *longname,
                  const wchar_t *shortname) {
    return (wcscmp(candidate, longname) == 0 ||
            wcscmp(candidate, shortname) == 0);
  }
  bool IsArgument(const wchar_t *candidate, const wchar_t *longname,
                  const wchar_t *shortname, const wchar_t *aliasname) {
    return (wcscmp(candidate, longname) == 0 ||
            wcscmp(candidate, shortname) == 0 ||
            wcscmp(candidate, aliasname) == 0);
  }
  bool IsArgument(const wchar_t *candidate, const wchar_t *argname,
                  size_t arglen, const wchar_t **off) {
    auto l = wcslen(candidate);
    if (l < arglen)
      return false;
    if (wcsncmp(candidate, argname, arglen) == 0) {
      if (l > arglen && candidate[arglen] == '=') {
        *off = candidate + arglen + 1;
        return true;
      }
      if (index_ + 1 < Argc_) {
        *off = Argv_[++index_];
        return true;
      }
    }
    return false;
  }
  int ProcessArgvImpl(KisasumFilelist &filelist) {
    const wchar_t *arg = nullptr;
    const wchar_t *argvalue = nullptr;
    for (; index_ < Argc_; index_++) {
      arg = Argv_[index_];
      if (arg[0] == '-') {
        if (IsArgument(arg, L"-h", L"--help", L"-?")) {
          KisasumUsage();
          ExitProcess(0);
        }
        if (IsArgument(arg, L"-v", L"--version")) {
          printf("1.0\n");
          ExitProcess(0);
        }
        if (IsArgument(arg, L"--format", sizeof("--format") - 1, &argvalue) ||
            IsArgument(arg, L"-f", sizeof("-f") - 1, &argvalue)) {
          filelist.format = KisasumFormatResolve(argvalue);
          continue;
        }
        if (IsArgument(arg, L"--algorithm", sizeof("--algorithm") - 1,
                       &argvalue) ||
            IsArgument(arg, L"-a", sizeof("-a") - 1, &argvalue)) {
          //
          filelist.algorithm = argvalue;
          continue;
        }
        console::Print(console::fc::Red, L"Invalid Argument%s\n", arg);
        return 1;
      } else {
        filelist.files.push_back(arg);
      }
    }
    if (filelist.algorithm.empty()) {
      filelist.algorithm.assign(L"sha256");
    }
    return 0;
  }

private:
  int Argc_;
  wchar_t **Argv_;
  int index_{1};
};

int ProcessArgv(int Argc, wchar_t **Argv, KisasumFilelist &filelist) {
  ArgvImpl argvimpl(Argc, Argv);
  return argvimpl.ProcessArgvImpl(filelist);
}