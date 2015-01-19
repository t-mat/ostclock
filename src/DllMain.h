#ifndef DLL_MAIN_H
#define DLL_MAIN_H

#ifndef DLL_MAIN_DECLSPEC
#  define DLL_MAIN_DECLSPEC
#endif

extern "C" void DLL_MAIN_DECLSPEC WINAPI HookEnd();
extern "C" void DLL_MAIN_DECLSPEC WINAPI HookStart(HWND hwndMain, HINSTANCE hModule);

struct SharedVariable {
    HWND    hwndMain { nullptr };
    HHOOK   hHook { nullptr };
    HWND    hwndClock { nullptr };
};

using AccessSharedVariableFunc = std::function<void(SharedVariable&)>;
bool accessSharedVariable(const AccessSharedVariableFunc& func);

#endif
