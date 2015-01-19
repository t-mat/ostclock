#ifndef DLL_MAIN_H
#define DLL_MAIN_H

#ifndef DLL_MAIN_DECLSPEC
#  define DLL_MAIN_DECLSPEC
#endif

extern "C" void DLL_MAIN_DECLSPEC WINAPI HookEnd();
extern "C" void DLL_MAIN_DECLSPEC WINAPI HookStart(HWND hwndMain, HINSTANCE hModule);

#endif
