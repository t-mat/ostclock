#ifndef UTIL_H
#define UTIL_H

void forceForegroundWindow(HWND hWnd);
bool checkWindowClassName(HWND hWnd, const TCHAR* windowClassName);
HWND findTaskbar();
HWND findTrayServer();
HWND findTrayClock();
void errorMessageBox(const TCHAR* msg, ...);

template <unsigned N = 256>
std::array<TCHAR, N> formatString(const TCHAR* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	std::array<TCHAR, N> buf;
	vswprintf_s(buf.data(), buf.size(), fmt, args);
	va_end(args);
	return buf;
}

#endif
