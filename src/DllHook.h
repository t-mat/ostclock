#ifndef DLL_HOOK_H
#define DLL_HOOK_H

enum {
    WM_CLOCK_REFRESH_CLOCK      = WM_USER,
    WM_CLOCK_REFRESH_TASKBAR,
    WM_CLOCK_END_CLOCK,
};

LRESULT dllHookCallback(int nCode, WPARAM wParam, LPARAM lParam);

#endif
