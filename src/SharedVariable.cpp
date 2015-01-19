#include "stdafx.h"
#include "SharedVariable.h"
#include "FileMapping.h"


// note : This function is called both T-Clock and Explore's process.
//        Callers are HookStart(), HookEnd(), dllHookCallback().
//
bool accessSharedVariable(const AccessSharedVariableFunc& func) {
    bool result = false;

    FileMapping::access(
          APP_SHARED_MEMORY_NAME
        , sizeof(SharedVariable)
        , [&](void* ptr, size_t) {
            auto* p = reinterpret_cast<SharedVariable*>(ptr);
            func(*p);
            result = true;
        }
    );

    return result;
}
