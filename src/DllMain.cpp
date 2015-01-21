#include "stdafx.h"
#define DLL_MAIN_DECLSPEC __declspec(dllexport)
#include "DllMain.h"
#include "DllHook.h"
#include "WinMain.h"
#include "SharedVariable.h"
#include "Util.h"

#pragma comment(lib, "UxTheme.lib")


BOOL WINAPI DllMain(HANDLE, DWORD, LPVOID) {
    return TRUE;
}


extern "C" __declspec(dllexport) void WINAPI HookStart(HWND hwndMain, HINSTANCE hModule) {
    // note : Do not access non-shared variables and
    //        related functions (endClock(), etc) from here.
    //
    // Since HookStart() and HookEnd() are running on T-Clock main window's process,
    // we should not manipulate tasktray clock's resources (hdcClock, etc).
    // Because tasktray clock is running on the Windows Explore's process.
    //

    const auto hwndBar       = findTaskbar();
    const auto hwndTrayClock = findTrayClock();
    const auto hThread       = GetWindowThreadProcessId(hwndBar, nullptr);

    bool hooked = false;

    if(hwndBar && hwndTrayClock && hThread) {
        accessSharedVariable([&](SharedVariable& sv) {
            sv.hwndMain = hwndMain;

            // note : In SetWindowsHookEx(),
            //        Explore's process (hThread) will load this DLL again.
            //        After loading the dll, dllHookCallback() intercepts
            //        all Explore's WndProc() messages.
            sv.hHook = SetWindowsHookEx(
                  WH_CALLWNDPROC
                , dllHookCallback
                , reinterpret_cast<HINSTANCE>(hModule)
                , hThread
            );
            hooked = true;
        });
    }

    if(!hooked) {
        sendMessage(hwndMain, WM_DLL_TO_MAINWND_ERROR);
    } else {
        postMessage(hwndBar, WM_SIZE, SIZE_RESTORED);
        sendMessage(hwndTrayClock, WM_NULL);
    }
}


extern "C" __declspec(dllexport) void WINAPI HookEnd() {
    // note : Do not access non-shared variables and
    //        related functions (endClock(), etc) from here.
    //
    // See HookStart() for the details.

    SharedVariable tmpSv {};
    if(accessSharedVariable([&](SharedVariable& sv) {
        tmpSv = sv;
        sv.hwndClock = nullptr;
        sv.hHook = nullptr;
    })) {
        // note : Unhook first !
        //        Until unhooked, dllHookCallback() is always watching
        //        Main window and taskbar.
        if(tmpSv.hHook) {
            UnhookWindowsHookEx(tmpSv.hHook);
        }
        if(isWindow(tmpSv.hwndClock)) {
            // Call endClock(). But, since we can't call endClock() directly,
            // we should send message to the clock window which is running
            // in the Windows Explore process.
            postMessage(tmpSv.hwndClock, WM_CLOCK_END_CLOCK);
        }
    }

    if(auto h = findTaskbar()) {
        postMessage(h, WM_SIZE, SIZE_RESTORED);
        InvalidateRect(h, nullptr, TRUE);
    }
}
