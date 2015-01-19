#include "stdafx.h"
#include "OutputDebugString.h"

#if defined(OUTPUT_DEBUG_STRING_H_OUTPUT_DEBUG_STRING_ENABLE)
static bool bEnable = true;

bool isEnableOutputDebugString() {
    return bEnable;
}

void enableOutputDebugString(bool enable) {
    bEnable = enable;
}

void outputDebugString(const wchar_t* fmt, ...) {
    if(isEnableOutputDebugString()) {
        va_list args;
        va_start(args, fmt);
        wchar_t buf[1024];
        vswprintf_s(buf, sizeof(buf)/sizeof(buf[0]), fmt, args);
        OutputDebugStringW(buf);
        va_end(args);
    }
}

void outputDebugString(const char* fmt, ...) {
    if(isEnableOutputDebugString()) {
        va_list args;
        va_start(args, fmt);
        char buf[1024];
        vsprintf_s(buf, sizeof(buf)/sizeof(buf[0]), fmt, args);
        OutputDebugStringA(buf);
        va_end(args);
    }
}
#endif
