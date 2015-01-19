#include "stdafx.h"
#define DLL_MAIN_DECLSPEC __declspec(dllexport)
#include "DllMain.h"
#include "DllHook.h"
#include "WinMain.h"
#include "FileMapping.h"
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

    bool b = false;
    SharedVariable tmpSv {};
    accessSharedVariable([&](SharedVariable& sv) {
        tmpSv = sv;
        sv.hwndClock = nullptr;
        sv.hHook = nullptr;
        b = true;
    });

    if(b) {
        if(isWindow(tmpSv.hwndClock)) {
            postMessage(tmpSv.hwndClock, WM_CLOCK_END_CLOCK);
        }
        if(tmpSv.hHook) {
            UnhookWindowsHookEx(tmpSv.hHook);
        }
    }

    if(auto h = findTaskbar()) {
        postMessage(h, WM_SIZE, SIZE_RESTORED);
        InvalidateRect(h, nullptr, TRUE);
    }
}


// note : This function is called both T-Clock and Explore's process.
//        Callers are HookStart(), HookEnd(), dllHookCallback().
//
bool accessSharedVariable(const AccessSharedVariableFunc& func) {
    bool result = false;

    FileMapping::access(
          APP_SHARED_MEMORY_NAME
        , sizeof(SharedVariable)
        , [&](void* ptr, size_t) {
            auto* p = reinterpret_cast<SharedVariable*>(ptr);
            func(*p);
            result = true;
        }
    );

    return result;
}
