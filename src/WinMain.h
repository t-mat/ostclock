﻿#ifndef WIN_MAIN_H
#define WIN_MAIN_H

enum {
    WM_DLL_TO_MAINWND_REGISTER_HWND = WM_USER,
    WM_DLL_TO_MAINWND_ERROR,
    WM_DLL_TO_MAINWND_EXIT,
    WM_DLL_TO_MAINWND_CONTEXTMENU,
    WM_DLL_TO_MAINWND_LEFT_CLICK,
};

LRESULT sendMessageToTasktrayClock(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

#endif
