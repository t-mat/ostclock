#ifndef BITMAP_H
#define BITMAP_H

struct Bitmap {
    Bitmap() {}

    ~Bitmap() {
        destroy();
    }

    bool good() const {
        return hBitmap != nullptr;
    }

    operator bool() const {
        return good();
    }

    bool create(HDC hdc, int x, int y) {
        destroy();
        hBitmap = CreateCompatibleBitmap(hdc, x, y);
        return good();
    }

    bool create(HDC hdc, const RECT& rect) {
        return create(hdc, rect.right, rect.bottom);
    }

    void destroy() {
        if(hBitmap) {
            DeleteObject(hBitmap);
            hBitmap = nullptr;
        }
    }

    void select(HDC hdc) const {
        if(good()) {
            SelectObject(hdc, hBitmap);
        }
    }

    BITMAP getBitmap() const {
        return getObject<BITMAP>(hBitmap);
    }

protected:
    HBITMAP hBitmap { nullptr };
};

#endif
