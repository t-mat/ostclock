#ifndef HDC_H
#define HDC_H

struct Hdc {
    Hdc() {}

    Hdc(const Hdc&) = delete;
    Hdc& operator=(Hdc&) = delete;

    Hdc(Hdc&& hdc) {
        this->gdiHdc = hdc.gdiHdc;
        this->hwndGetDc = hdc.hwndGetDc;

        hdc.gdiHdc = nullptr;
        hdc.hwndGetDc = InvalidHwndValue;
    }

    ~Hdc() {
        destroy();
    }

    operator HDC() const {
        return gdiHdc;
    }

    void getDc_(HWND hwnd) {
        destroy();
        hwndGetDc = hwnd;
        gdiHdc = GetDC(hwnd);
    }

    static Hdc getDc(HWND hwnd) {
        Hdc hdc;
        hdc.getDc_(hwnd);
        return hdc;
    }

    void createCompatibleDc_(HDC hdc) {
        destroy();
        gdiHdc = CreateCompatibleDC(hdc);
    }

    static Hdc createCompatibleDc(HDC cHdc) {
        Hdc hdc;
        hdc.createCompatibleDc_(cHdc);
        return hdc;
    }

    void destroy() {
        if(gdiHdc) {
            if(InvalidHwndValue != hwndGetDc) {
                ReleaseDC(hwndGetDc, gdiHdc);
                hwndGetDc = InvalidHwndValue;
            } else {
                DeleteDC(gdiHdc);
            }
            gdiHdc = nullptr;
        }
    }

protected:
    const HWND InvalidHwndValue = reinterpret_cast<HWND>(-1);

    HDC gdiHdc { nullptr };
    HWND hwndGetDc { InvalidHwndValue };
};

#endif
