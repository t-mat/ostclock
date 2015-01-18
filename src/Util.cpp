#include "stdafx.h"
#include "Util.h"

namespace {
const TCHAR strShellTrayWnd[] = _T("Shell_TrayWnd");
const TCHAR strTrayNotifyWnd[] = _T("TrayNotifyWnd");
const TCHAR strTrayClockWClass[] = _T("TrayClockWClass");
const TCHAR strCTrayServer[] = _T("CTrayServer");
}


void forceForegroundWindow(HWND hWnd) {
	DWORD pid = 0;
	auto thread1 = GetWindowThreadProcessId(GetForegroundWindow(), &pid);
	auto thread2 = GetCurrentThreadId();

	AttachThreadInput(thread2, thread1, TRUE);
	SetForegroundWindow(hWnd);

	AttachThreadInput(thread2, thread1, FALSE);
	BringWindowToTop(hWnd);
}


bool checkWindowClassName(HWND hWnd, const TCHAR* windowClassName) {
	bool result = false;

	TCHAR classname[256] {};
	if(GetClassName(hWnd, classname, _countof(classname)) > 0) {
		result = (_tcsicmp(classname, windowClassName) == 0);
	}

	return result;
}


HWND findTaskbar() {
	return FindWindow(strShellTrayWnd, nullptr);
}


HWND findTrayServer() {
	return FindWindow(strShellTrayWnd, strCTrayServer);
}


static HWND findTrayNotify() {
	HWND h = findTaskbar();
	for(h = GetWindow(h, GW_CHILD); h; h = GetWindow(h, GW_HWNDNEXT)) {
		if(checkWindowClassName(h, strTrayNotifyWnd)) {
			return h;
		}
	}
	return nullptr;
}


HWND findTrayClock() {
	HWND h = findTrayNotify();
	for(;;) {
		h = GetWindow(h, GW_CHILD);
		if(!h || checkWindowClassName(h, strTrayClockWClass)) {
			return h;
		}
	}
}


void errorMessageBox(const TCHAR* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	TCHAR buf[4096];
	_vstprintf_s(buf, fmt, args);
	MessageBox(nullptr, buf, _T(APPNAME), MB_OK | MB_ICONEXCLAMATION);
	va_end(args);
}
