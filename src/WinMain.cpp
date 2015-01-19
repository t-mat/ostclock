#include "stdafx.h"
#include "DllLoader.h"
#include "FileMapping.h"
#include "EnsureSingleInstance.h"
#include "PopupMenu.h"
#include "TaskbarMessage.h"
#include "DllHook.h"
#include "DllMain.h"
#include "Util.h"
#include "PropertySheet.h"
#include "WinMain.h"

#pragma comment(lib, "comctl32.lib")

#pragma comment(linker, "/manifestdependency:\"" \
    "type='win32'"                               \
    " name='Microsoft.Windows.Common-Controls'"  \
    " version='6.0.0.0'"                         \
    " processorArchitecture='amd64'"             \
    " publicKeyToken='6595b64144ccf1df'"         \
    " language='*'"                              \
"\"")

namespace {

DllLoader               dllLoader {};
TCHAR                   dllFilename[] = _T(APP_DLL_NAME);
decltype(HookStart)*    hookStart = nullptr;
decltype(HookEnd)*      hookEnd = nullptr;
HWND                    hwndTasktrayClock = nullptr;
UINT                    taskbarRestartMessage = 0;

enum : WORD {
    IDC_SHOW_DATE_AND_CLOCK,// Left click : Show date and clock dialog
    IDC_ADJUST_DATETIME,    // Menu : Adjust Date & Time
    IDC_TASKMANAGER,        // Menu : Task Manager
    IDC_SHUTDOWN,           // Menu : Exit Windows / Shutdown
    IDC_REBOOT,             // Menu : Exit Windows / Reboot
    IDC_LOGOFF,             // Menu : Exit Windows / Logoff
    IDC_SHOWPROP,           // Menu : Properties
    IDC_REFRESH,            // Menu : Reresh
    IDC_EXIT,               // Menu : Exit
};


LRESULT onCommand(HWND, WORD cmd) {
    switch(cmd) {
    default:
        break;

    case IDC_REFRESH:
        if(auto h = findTrayClock()) {
            sendMessage(h, WM_CLOCK_REFRESH_CLOCK);
            sendMessage(h, WM_CLOCK_REFRESH_TASKBAR);
        }
        break;

    case IDC_SHOWPROP:
        openPropertySheet();
        break;

    case IDC_EXIT:
        if(hwndTasktrayClock) {
            postMessage(hwndTasktrayClock, WM_CLOCK_END_CLOCK);
        }
        break;

    case IDC_SHUTDOWN:
        if(! ExitWindowsEx(EWX_SHUTDOWN|EWX_FORCE, 0)) {
            errorMessageBox(_T("Shutdown request failed"));
        }
        break;

    case IDC_REBOOT:
        if(! ExitWindowsEx(EWX_REBOOT|EWX_FORCE, 0)) {
            errorMessageBox(_T("Reboot request failed"));
        }
        break;

    case IDC_LOGOFF:
        if(! ExitWindowsEx(EWX_LOGOFF|EWX_FORCE, 0)) {
            errorMessageBox(_T("Logoff request failed"));
        }
        break;

    case IDC_ADJUST_DATETIME:
        if(auto h = findTaskbar()) {
            postMessage(h, WM_COMMAND, TASKBAR_MSG_DATE_AND_TIME);
        }
        break;

    case IDC_TASKMANAGER:
        if(auto h = findTaskbar()) {
            postMessage(h, WM_COMMAND, TASKBAR_MSG_TASK_MANAGER);
        }
        break;

    case IDC_SHOW_DATE_AND_CLOCK:
        if(auto h = findTrayClock()) {
            postMessage(h, TRAYCLOCK_MSG_CALENDAR_AND_CLOCK, 1, 0);
        }
    }

    return 0;
}


LRESULT onContextMenu(HWND hwnd) {
    POINT p {};
    GetCursorPos(&p);

    PopupMenu subMenu {};
    subMenu.append(MF_STRING, IDC_SHUTDOWN, _T("Shutdown"));
    subMenu.append(MF_STRING, IDC_REBOOT, _T("Reboot"));
    subMenu.append(MF_STRING, IDC_LOGOFF, _T("Logoff"));

    PopupMenu mainMenu {};
    mainMenu.append(MF_STRING, IDC_SHOW_DATE_AND_CLOCK, _T("&Show Date && Clock"));
    mainMenu.append(MF_STRING, IDC_ADJUST_DATETIME, _T("&Adjust Date/Time"));
    mainMenu.append(MF_STRING, IDC_TASKMANAGER, _T("&Task Manager"));
    mainMenu.append(MF_STRING | MF_POPUP, subMenu.hMenu, _T("Exit Windows"));
    mainMenu.append(MF_SEPARATOR);
    mainMenu.append(MF_STRING, IDC_SHOWPROP, _T("&Properties"));
    mainMenu.append(MF_STRING, IDC_REFRESH, _T("&Refresh"));
    mainMenu.append(MF_SEPARATOR);
    mainMenu.append(MF_STRING, IDC_EXIT, _T("E&xit"));

    forceForegroundWindow(hwnd);
    mainMenu.track(hwnd, p.x, p.y, TPM_NONOTIFY|TPM_LEFTBUTTON);

    return 0;
}


LRESULT CALLBACK
wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if(uMsg == taskbarRestartMessage) {
        hookEnd();
        hookStart(hwnd, dllLoader.getHinstance());
    }

    switch(uMsg) {
    case WM_CREATE:
        hookStart(hwnd, dllLoader.getHinstance());
        return 0;

    case WM_DESTROY:
        hookEnd();
        PostQuitMessage(0);
        return 0;

    case WM_ENDSESSION:
        if(wParam) {
            hookEnd();
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
        }
        return 0;

    case WM_WININICHANGE:
    case WM_SYSCOLORCHANGE:
        if(auto h = findTrayClock()) {
            sendMessage(h, WM_CLOCK_REFRESH_TASKBAR);
        }
        return 0;

    case WM_COMMAND:
        return onCommand(hwnd, LOWORD(wParam));

    case WM_DLL_TO_MAINWND_ERROR:
        {
            auto x = formatString(_T("Failed to customize tasktray clock.\r\n(ERROR CODE #%d)"), lParam);
            errorMessageBox(x.data());
        }
        postMessage(hwnd, WM_CLOSE);
        return 0;

    case WM_DLL_TO_MAINWND_REGISTER_HWND:
        hwndTasktrayClock = reinterpret_cast<HWND>(lParam);
        return 0;

    case WM_DLL_TO_MAINWND_EXIT:
        closePropertySheet();
        postMessage(hwnd, WM_CLOSE);
        return 0;

    case WM_DLL_TO_MAINWND_CONTEXTMENU:
        return onContextMenu(hwnd);

    case WM_DLL_TO_MAINWND_LEFT_CLICK:
        postMessage(hwnd, WM_COMMAND, IDC_SHOW_DATE_AND_CLOCK);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

} // Anonymous namespace


LRESULT sendMessageToTasktrayClock(UINT message, WPARAM wParam, LPARAM lParam) {
    if(hwndTasktrayClock) {
        return sendMessage(hwndTasktrayClock, message, wParam, lParam);
    } else {
        return 0;
    }
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    EnsureSingleInstance ensureSingleInstance { _T("Local\\") _T(APPNAME) };
    if(! ensureSingleInstance.good()) {
        return 0;
    }

    FileMapping fileMapping { APP_SHARED_MEMORY_NAME, sizeof(SharedVariable) };

    dllLoader.load(dllFilename);
    dllLoader.getProcAddress(hookStart, "HookStart");
    dllLoader.getProcAddress(hookEnd, "HookEnd");
    if(! dllLoader.good() || ! hookStart || ! hookEnd) {
        errorMessageBox(L"Failed to load DLL : %s", dllFilename);
        return 0;
    }

    if(HWND hwndTrayServer = findTrayServer()) {
        sendMessage(hwndTrayServer, WM_CLOSE);
    }

    // Message of the taskbar recreating - Special thanks to Mr.Inuya
    taskbarRestartMessage = RegisterWindowMessage(_T("TaskbarCreated"));

    WNDCLASS wc {};
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = wndProc;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = _T(APPNAME) _T("MainWindowClass");
    RegisterClass(&wc);

    CreateWindowEx(WS_EX_TOOLWINDOW, wc.lpszClassName, _T(APPNAME),
                    0, 0, 0, 0, 0, 0, 0, wc.hInstance, nullptr);

    MSG msg;
    while(GetMessage(&msg, nullptr, 0, 0)) {
        if(! isPropertySheetDialogMessage(&msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    dllLoader.unload();
    return 0;
}
