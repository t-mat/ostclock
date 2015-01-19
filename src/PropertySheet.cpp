#include "stdafx.h"
#include "ResourcelessDialog.h"
#include "PropertySheet.h"
#include "PropertySheetPageColor.h"
#include "DllHook.h"
#include "WinMain.h"
#include "Util.h"


namespace {

HWND    hwndPropertySheet { nullptr };


LRESULT propertySheetSubClassProc(
      HWND hwnd
    , UINT uMsg
    , WPARAM wParam
    , LPARAM lParam
) {
    auto oldWndProc = getWindowLongPtr<WNDPROC>(hwnd, GWLP_USERDATA);
    const auto l = CallWindowProc(oldWndProc, hwnd, uMsg, wParam, lParam);

    switch(uMsg) {
    default:
        break;

    case WM_COMMAND:
        {
            const WORD id = LOWORD(wParam);

            if(id == IDOK || id == IDCANCEL) {
                DestroyWindow(hwnd);
                hwndPropertySheet = nullptr;
            }

            if(id == IDOK || id == ID_APPLY_NOW) {
                // apply settings
                sendMessageToTasktrayClock(WM_CLOCK_REFRESH_CLOCK);
                sendMessageToTasktrayClock(WM_CLOCK_REFRESH_TASKBAR);
            }

            EmptyWorkingSet(GetCurrentProcess());
        }
        break;

    case WM_SYSCOMMAND:
        if((wParam & 0xfff0) == SC_CLOSE) {
            postMessage(hwnd, WM_COMMAND, IDCANCEL);
        }
        break;
    }

    return l;
}


int CALLBACK propertySheetProc(HWND hDlg, UINT uMsg, LPARAM) {
    if(PSCB_INITIALIZED == uMsg) {
        auto* p = getWindowLongPtr<WNDPROC>(hDlg, GWLP_WNDPROC);
        if(p != propertySheetSubClassProc) {
            setWindowLongPtr(hDlg, GWLP_USERDATA, p);
            setWindowLongPtr(hDlg, GWLP_WNDPROC, propertySheetSubClassProc);
        }
    }
    return 0;
}

} //namespace


void openPropertySheet() {
    if(isWindow(hwndPropertySheet)) {
        forceForegroundWindow(hwndPropertySheet);
        return;
    }

    auto rds = std::vector<ResourcelessDialog> {
        createColorDlg()
    };

    hwndPropertySheet = ResourcelessDialog::propertySheet(
          rds
        , _T(APPNAME)
        , propertySheetProc
    );
    forceForegroundWindow(hwndPropertySheet);
}


void closePropertySheet() {
    if(isWindow(hwndPropertySheet)) {
        postMessage(hwndPropertySheet, WM_CLOSE);
    }
    hwndPropertySheet = nullptr;
}


bool isPropertySheetDialogMessage(MSG* msg) {
    bool result = false;
    if(isWindow(hwndPropertySheet)) {
        result = (IsDialogMessage(hwndPropertySheet, msg) != 0);
    }
    return result;
}
