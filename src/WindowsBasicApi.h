#ifndef WINDOWS_BASIC_API_H
#define WINDOWS_BASIC_API_H

template<typename T_RESULT = LRESULT, typename T_WPARAM = WPARAM, typename T_LPARAM = LPARAM>
static T_RESULT sendMessage(HWND hWnd, UINT uMsg, T_WPARAM wParam = (T_WPARAM) 0, T_LPARAM lParam = (T_LPARAM) 0) {
	return (T_RESULT) SendMessage(hWnd, uMsg, (WPARAM) wParam, (LPARAM) lParam);
}

template<typename T_WPARAM = WPARAM, typename T_LPARAM = LPARAM>
static BOOL postMessage(HWND hWnd, UINT uMsg, T_WPARAM wParam = (T_WPARAM) 0, T_LPARAM lParam = (T_LPARAM) 0) {
	return PostMessage(hWnd, uMsg, (WPARAM) wParam, (LPARAM) lParam);
}

template<typename T_RESULT = LRESULT, typename T_WPARAM = WPARAM, typename T_LPARAM = LPARAM>
static T_RESULT sendDlgItemMessage(HWND hWnd, int idControl, UINT uMsg, T_WPARAM wParam = (T_WPARAM)0, T_LPARAM lParam = (T_LPARAM)0) {
	return (T_RESULT) SendDlgItemMessage(hWnd, idControl, uMsg, (WPARAM)(wParam), (LPARAM)(lParam));
}

template<typename T_NEWITEMID = UINT_PTR, typename T_NEWITEMPTR = LPCTSTR>
static BOOL appendMenu(HMENU hMenu, UINT uFlags, T_NEWITEMID uIDNewItem, T_NEWITEMPTR lpNewItem) {
	return AppendMenu(hMenu, uFlags, (UINT_PTR) uIDNewItem, (LPCTSTR) lpNewItem);
}


template<typename T = LONG_PTR>
T getWindowLongPtr(HWND hWnd, int nIndex) {
	return (T) GetWindowLongPtr(hWnd, nIndex);
}

template<typename T = LONG_PTR>
T setWindowLongPtr(HWND hWnd, int nIndex, T lNewLong) {
	return (T) SetWindowLongPtr(hWnd, nIndex, (LONG_PTR) lNewLong);
}

template<typename T>
T getObject(HGDIOBJ hGdiObj) {
	T t {};
	GetObject(hGdiObj, sizeof(t), &t);
	return t;
}


inline bool isWindow(HWND hWnd) {
	return hWnd && IsWindow(hWnd);
}


#endif
