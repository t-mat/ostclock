#include "stdafx.h"
#include "DateTimeFormat.h"
#include "Timer.h"
#include "Hdc.h"
#include "Font.h"
#include "Bitmap.h"
#include "Registry.h"
#include "Config.h"
#include "WinMain.h"
#include "DllMain.h"
#include "DllHook.h"
#include "SharedVariable.h"
#include "Util.h"
#include "TaskbarMessage.h"

// note : All functions and variables in this file are running on
//        the Windows Explore's process.
//

namespace {

Config  config {};
Timer   timer {};
Font    font {};
Bitmap  clockBitmap;
Hdc     hdcClock;
WNDPROC oldWndProc { nullptr };
HHOOK   hHook { nullptr };
HWND    hwndClock { nullptr };
HWND    hwndMain { nullptr };

int     iClockWidth { -1 };

RECT getClientRect(HWND hWnd) {
    RECT rc;
    GetClientRect(hWnd, &rc);
    return rc;
}


TEXTMETRIC getTextMetrics(HDC hdc) {
    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    return tm;
}


SYSTEMTIME getDisplayTime() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    return st;
}


POINT drawText(
      HDC hdc
    , TCHAR* text
    , bool bDraw
    , int xOfs
    , int yOfs
    , int yOfs2
    , int lineSpacing
) {
    const auto tm = getTextMetrics(hdc);
    const int hf = tm.tmHeight - tm.tmInternalLeading;
    int w = 0;
    int y = hf / 4 - tm.tmInternalLeading / 2;
    for(auto* p = text; *p; ) {
        auto* sp = p;
        while(*p && *p != 0x0a) {
            p++;
        }
        if(*p == 0x0a) {
            *p++ = 0;
        }

        if(bDraw) {
            if(*p == 0 && sp == text) {
                y = (yOfs - tm.tmHeight) / 2  - tm.tmInternalLeading / 4;
            }
            TextOut(hdc, xOfs, y + yOfs2, sp, (int)_tcslen(sp));
        }

        const auto len = static_cast<int>(_tcslen(sp));
        SIZE sz {};
        if(GetTextExtentPoint32(hdc, sp, len, &sz) == 0) {
            sz.cx = len * tm.tmAveCharWidth;
        }
        if(w < sz.cx) {
            w = sz.cx;
        }
        y += hf;
        if(*p) {
            y += 2 + lineSpacing;
        }
    }
    w += tm.tmAveCharWidth * 2;
    return { w, y };
}


void destroyClockDc() {
    hdcClock.destroy();
    clockBitmap.destroy();
}


void deleteClockRes() {
    font.close();
    destroyClockDc();
    timer.killTimer();
}


void endClock() {
    deleteClockRes();

    if(isWindow(hwndClock)) {
        if(oldWndProc) {
            setWindowLongPtr(hwndClock, GWLP_WNDPROC, oldWndProc);
        }
        // note : Do not clear oldWndProc like this :
        //           oldWndProc = nullptr;
        //        oldWndProc has role as 'hook at once' flag in dllHookCallback().
    }

    if(isWindow(hwndMain)) {
        postMessage(hwndMain, WM_DLL_TO_MAINWND_EXIT);
    }
}


void drawClockSub(HWND hwnd, HDC hdc, const SYSTEMTIME& pt) {
    if(!hdcClock || !clockBitmap) {
        destroyClockDc();
        if(auto hdc = Hdc::getDc(nullptr)) {
            hdcClock.createCompatibleDc_(hdc);
            if(hdcClock) {
                if(clockBitmap.create(hdc, getClientRect(hwnd))) {
                    clockBitmap.select(hdcClock);
                    font.select(hdcClock);
                    SetBkMode   (hdcClock, TRANSPARENT);
                    SetTextAlign(hdcClock, TA_CENTER|TA_TOP);
                    SetTextColor(hdcClock, config.text.color.foreground);
                }
            }
        }
    }

    if(!hdcClock || !clockBitmap) {
        return;
    }

    const auto bmp = clockBitmap.getBitmap();

    if(config.explore.isXpStyle) {
        DrawThemeParentBackground(hwnd, hdcClock, nullptr);
    } else {
        RECT rc {};
        rc.right = bmp.bmWidth;
        rc.bottom = bmp.bmHeight;
        HBRUSH hbr = CreateSolidBrush(config.text.color.background);
        FillRect(hdcClock, &rc, hbr);
        DeleteObject(hbr);
    }

    SetTextColor(hdc, config.text.color.foreground);
    const RECT rcClock = getClientRect(hwnd);
    auto f = makeDateTimeString(pt, config.text.format.data());
    const auto ept = drawText(
          hdcClock
        , f.data()
        , true
        , config.text.xPos + rcClock.right / 2
        , rcClock.bottom
        , config.text.yPos
        , config.text.lineSpacing
    );

    BitBlt(hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcClock, 0, 0, SRCCOPY);

    const auto w = ept.x + config.text.width;
    if(w > iClockWidth) {
        iClockWidth = w;
        postMessage(GetParent(GetParent(hwnd)), WM_SIZE, SIZE_RESTORED, 0);
    }
}


void onTimer(HWND hwnd, WPARAM) {
    const auto t = getDisplayTime();
    timer.update(t);
    if(auto hdc = Hdc::getDc(hwnd)) {
        drawClockSub(hwnd, hdc, t);
    }
}


LRESULT onCalcRect(HWND hwnd, const SYSTEMTIME& pt) {
    if(!(getWindowLongPtr(hwnd, GWL_STYLE) & WS_VISIBLE)) {
        return 0;
    }

    if(auto hdc = Hdc::getDc(hwnd)) {
        font.select(hdc);

        auto f = makeDateTimeString(pt, config.text.format.data());
        const auto ept = drawText(
              hdc
            , f.data()
            , false
            , config.text.xPos
            , 0
            , config.text.yPos
            , config.text.lineSpacing
        );

        const auto tm = getTextMetrics(hdc);
        const int hf = tm.tmHeight - tm.tmInternalLeading;
        int w = ept.x;
        int h = ept.y + hf / 2;

        if(iClockWidth < 0) {
            iClockWidth = w;
        } else {
            w = iClockWidth;
        }
        w += config.text.width;
        h += config.text.height;

        w = std::max<int>(8, w);
        h = std::max<int>(8, h);

        return MAKELONG(w, h);
    }

    return 0;
}


LRESULT CALLBACK
wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
    default:
        break;

    case WM_DESTROY:
        deleteClockRes();
        break;

    case TRAYCLOCK_MSG_CALC_RECT:
        return onCalcRect(hwnd, getDisplayTime());

    case WM_ERASEBKGND:
        return 0;

    case WM_SYSCOLORCHANGE:
        config = loadConfig();
        //break; // fall through
    case WM_TIMECHANGE:
        destroyClockDc();
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            drawClockSub(hwnd, hdc, getDisplayTime());
            EndPaint(hwnd, &ps);
        }
        return 0;

    case WM_TIMER:
        onTimer(hwnd, wParam);
        return 0;

    case WM_MOUSEMOVE:
        return 0;

    case WM_LBUTTONDOWN:
        postMessage(hwndMain, WM_DLL_TO_MAINWND_LEFT_CLICK);
        return 0;

    case WM_CONTEXTMENU:
        postMessage(hwndMain, WM_DLL_TO_MAINWND_CONTEXTMENU);
        return 0;

    case WM_NCHITTEST:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    case WM_MOUSEACTIVATE:
        return MA_ACTIVATE;

    case WM_WINDOWPOSCHANGING:
        if(auto* pwp = reinterpret_cast<LPWINDOWPOS>(lParam)) {
            if(IsWindowVisible(hwnd) && !(pwp->flags & SWP_NOSIZE)) {
                const auto h = HIWORD(onCalcRect(hwnd, getDisplayTime()));
                if(pwp->cy > h) {
                    pwp->cy = h;
                }
            }
        }
        break;

    case WM_CLOCK_REFRESH_CLOCK: // refresh the clock
        config = loadConfig();
        initDateTimeFormat();
        font.open(
              static_cast<WORD>(config.font.langId)
            , config.font.quality
            , config.font.name.data()
            , config.font.size
            , config.font.bold ? FW_BOLD : 0
            , config.font.italic
            , 0
        );
        timer.start(hwnd);
        iClockWidth = -1;
        destroyClockDc();
        InvalidateRect(hwnd, nullptr, FALSE);
        InvalidateRect(GetParent(hwnd), nullptr, TRUE);
        return 0;

    case WM_CLOCK_REFRESH_TASKBAR: // refresh other elements than clock
        destroyClockDc();
        postMessage(GetParent(GetParent(hwnd)), WM_SIZE, SIZE_RESTORED);
        InvalidateRect(GetParent(GetParent(hwnd)), nullptr, TRUE);
        return 0;

    case WM_CLOCK_END_CLOCK:
        endClock();
        return 0;
    }

    if(oldWndProc) {
        return CallWindowProc(oldWndProc, hwnd, uMsg, wParam, lParam);
    } else {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

} // Anonymous namespace


LRESULT dllHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    const auto pcwps = reinterpret_cast<LPCWPSTRUCT>(lParam);

    if(    nCode >= 0
        && nullptr == oldWndProc
        && pcwps
        && pcwps->hwnd
        && checkWindowClassName(pcwps->hwnd, _T("TrayClockWClass"))
    ) {
        const auto h = pcwps->hwnd;
        SharedVariable tmpSv {};

        if(accessSharedVariable([&](SharedVariable& sv) {
            tmpSv = sv;
            sv.hwndClock = h;
        })) {
            hwndClock   = h;
            hHook       = tmpSv.hHook;
            hwndMain    = tmpSv.hwndMain;

            if(hwndMain && hHook) {
                postMessage(hwndMain, WM_DLL_TO_MAINWND_REGISTER_HWND, 0, h);
                oldWndProc = getWindowLongPtr<WNDPROC>(h, GWLP_WNDPROC);
                setWindowLongPtr(h, GWLP_WNDPROC, wndProc);
                SetClassLong(h, GCL_STYLE, GetClassLong(h, GCL_STYLE) & ~CS_DBLCLKS);

                const auto pph = GetParent(GetParent(h));
                postMessage(pph, WM_SIZE, SIZE_RESTORED);
                InvalidateRect(pph, nullptr, TRUE);

                postMessage(h, WM_CLOCK_REFRESH_CLOCK);
                postMessage(h, WM_CLOCK_REFRESH_TASKBAR);
            }
        }
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}
