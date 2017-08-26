#ifndef KISASUM_CONSOLE_HPP
#define KISASUM_CONSOLE_HPP

#pragma once

#ifndef _WINDOWS_
#include <Windows.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <io.h>
#include <string>

namespace console {

namespace fc {
enum Color : WORD {
  Black = 0,
  DarkBlue = 1,
  DarkGreen = 2,
  DarkCyan = 3,
  DarkRed = 4,
  DarkMagenta = 5,
  DarkYellow = 6,
  Gray = 7,
  DarkGray = 8,
  Blue = 9,
  Green = 10,
  Cyan = 11,
  Red = 12,
  Magenta = 13,
  Yellow = 14,
  White = 15
};
}

namespace bc {
enum Color : WORD {
  Black = 0,
  DarkGray = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED,
  Blue = BACKGROUND_BLUE,
  Green = BACKGROUND_GREEN,
  Red = BACKGROUND_RED,
  Yellow = BACKGROUND_RED | BACKGROUND_GREEN,
  Magenta = BACKGROUND_RED | BACKGROUND_BLUE,
  Cyan = BACKGROUND_GREEN | BACKGROUND_BLUE,
  LightWhite = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED |
               BACKGROUND_INTENSITY,
  LightBlue = BACKGROUND_BLUE | BACKGROUND_INTENSITY,
  LightGreen = BACKGROUND_GREEN | BACKGROUND_INTENSITY,
  LightRed = BACKGROUND_RED | BACKGROUND_INTENSITY,
  LightYellow = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY,
  LightMagenta = BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
  LightCyan = BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
};
}

bool EnableVTMode();
int WriteConsoleInternal(const wchar_t *buffer, size_t len);
int WriteInternal(int color, const wchar_t *buf, size_t len);

template <typename T> T Argument(T value) noexcept { return value; }
template <typename T>
T const *Argument(std::basic_string<T> const &value) noexcept {
  return value.c_str();
}
template <typename... Args>
int StringPrint(wchar_t *const buffer, size_t const bufferCount,
                wchar_t const *const format, Args const &... args) noexcept {
  int const result = swprintf(buffer, bufferCount, format, Argument(args)...);
  // ASSERT(-1 != result);
  return result;
}

template <typename... Args>
int PrintConsole(const wchar_t *format, Args... args) {
  std::wstring buffer;
  size_t size = StringPrint(nullptr, 0, format, args...);
  buffer.resize(size);
  size = StringPrint(&buffer[0], buffer.size() + 1, format, args...);
  return WriteConsoleInternal(buffer.data(), size);
}

template <typename... Args>
int Print(int color, const wchar_t *format, Args... args) {
  std::wstring buffer;
  size_t size = StringPrint(nullptr, 0, format, args...);
  buffer.resize(size);
  size = StringPrint(&buffer[0], buffer.size() + 1, format, args...);
  return WriteInternal(color, buffer.data(), size);
}

size_t WriteFormatted(const wchar_t *data, size_t len);

template <typename... Args>
size_t PrintNone(const wchar_t *format, Args... args) {
	std::wstring buffer;
	size_t size = StringPrint(nullptr, 0, format, args...);
	buffer.resize(size);
	size = StringPrint(&buffer[0], buffer.size() + 1, format, args...);
	return WriteFormatted(buffer.data(), size);
}

class ProgressBar
{
public:
	ProgressBar() {
		text.reserve(256);
	}
	~ProgressBar() = default;
	void Update(std::uint32_t N) {
		size_t z = N / 2;
		size_t k = 50 - z;
		text.assign(L"\r[");
		if (z > 0) {
			text.append(z, '#');
		}
		if (N % 2 != 0) {
			text.push_back('>');
			k--;
		}
		if (k > 0) {
			text.append(k, ' ');
		}
		text.append(L"] ").append(std::to_wstring(N)).append(L"% completed.");
		WriteInternal(fc::Yellow,text.data(), text.size());
	}
	void Refresh() {
		std::wstring buf(text.size(), ' ');
		buf.front() = L'\r';
		WriteFormatted(buf.data(), buf.size());
	}
private:
	std::wstring text;
};
//console::ProgressBar bar;
//for (std::uint32_t i = 0; i <= 100; i++) {
//	bar.Update(i);
//	Sleep(50);
//}
//bar.Refresh();
//console::PrintNone(L"\rCompleted Progress..\n");
}
//

#endif