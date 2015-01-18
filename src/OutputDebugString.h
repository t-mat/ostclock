#ifndef OUTPUT_DEBUG_STRING_H
#define OUTPUT_DEBUG_STRING_H

#if defined(_DEBUG) || defined(OUTPUT_DEBUG_STRING_ENABLE)
#define OUTPUT_DEBUG_STRING_H_OUTPUT_DEBUG_STRING_ENABLE
bool isEnableOutputDebugString();
void enableOutputDebugString(bool enable);
void outputDebugString(const wchar_t* fmt, ...);
void outputDebugString(const char* fmt, ...);
#else
#define isEnableOutputDebugString(...) false
#define enableOutputDebugString(...)
#define outputDebugString(...)
#endif

#endif
