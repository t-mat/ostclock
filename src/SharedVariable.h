#ifndef SHARED_VARIABLE_H
#define SHARED_VARIABLE_H

struct SharedVariable {
    HWND    hwndMain { nullptr };
    HHOOK   hHook { nullptr };
    HWND    hwndClock { nullptr };
    TCHAR   mainExePath[MAX_PATH + 1] {};
};

using AccessSharedVariableFunc = std::function<void(SharedVariable&)>;
bool accessSharedVariable(const AccessSharedVariableFunc& func);

#endif
