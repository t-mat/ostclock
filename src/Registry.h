#ifndef REGISTRY_H
#define REGISTRY_H

using RegKeyFunc = std::function<void(HKEY)>;


inline void openRegKey(
      HKEY hKeyBase
    , const TCHAR* baseKey
    , const RegKeyFunc& regKeyFunc
) {
    HKEY hKey = nullptr;
    RegOpenKey(hKeyBase, baseKey, &hKey);
    if(hKey) {
        regKeyFunc(hKey);
        RegCloseKey(hKey);
    }
}


inline void createRegKey(
      HKEY hKeyBase
    , const TCHAR* baseKey
    , const RegKeyFunc& regKeyFunc
) {
    HKEY hKey = nullptr;
    RegCreateKey(hKeyBase, baseKey, &hKey);
    if(hKey) {
        regKeyFunc(hKey);
        RegCloseKey(hKey);
    }
}


inline bool setRegDword(
      HKEY hKeyBase
    , const TCHAR* baseKey
    , const TCHAR* subKey
    , DWORD val
) {
    LONG r = ERROR_INVALID_FUNCTION;
    const auto* p = reinterpret_cast<const BYTE*>(&val);
    const auto bytes = static_cast<DWORD>(sizeof(val));

    createRegKey(hKeyBase, baseKey, [&](HKEY hKey) {
        r = RegSetValueEx(hKey, subKey, 0, REG_DWORD, p, bytes);
    });

    return ERROR_SUCCESS == r;
}


inline bool setRegStr(
      HKEY hKeyBase
    , const TCHAR* baseKey
    , const TCHAR* subKey
    , const TCHAR* val
) {
    LONG r = ERROR_INVALID_FUNCTION;
    const auto* p = reinterpret_cast<const BYTE*>(val);
    const auto bytes = static_cast<DWORD>(_tcslen(val) * sizeof(*val));

    createRegKey(hKeyBase, baseKey, [&](HKEY hKey) {
        r = RegSetValueEx(hKey, subKey, 0, REG_SZ, p, bytes);
    });

    return ERROR_SUCCESS == r;
}


inline DWORD getRegDword(
      HKEY hKeyBase
    , const TCHAR* baseKey
    , const TCHAR* subKey
    , DWORD defaultVal = 0
) {
    DWORD result = 0;

    LONG r = ERROR_INVALID_FUNCTION;
    auto* p = reinterpret_cast<BYTE*>(&result);
    DWORD bytes = sizeof(result);

    openRegKey(hKeyBase, baseKey, [&](HKEY hKey) {
        r = RegQueryValueEx(hKey, subKey, 0, nullptr, p, &bytes);
    });

    if(ERROR_SUCCESS != r || sizeof(result) != bytes) {
        result = defaultVal;
    }
    return result;
}


template <class T, class U = int>
T getRegDword(
      HKEY hKeyBase
    , const TCHAR* baseKey
    , const TCHAR* subKey
    , U defaultVal = (U)0
) {
    return static_cast<T>(getRegDword(
          hKeyBase
        , baseKey
        , subKey
        , (DWORD) defaultVal
    ));
}


inline std::wstring getRegStr(
      HKEY hKeyBase
    , const TCHAR* baseKey
    , const TCHAR* subKey
    , const TCHAR* defaultVal = _T("")
) {
    std::array<TCHAR, 1024> result;

    LONG r = ERROR_INVALID_FUNCTION;
    auto* p = reinterpret_cast<BYTE*>(result.data());
    DWORD bytes = static_cast<DWORD>(result.size() * sizeof(result[0]));

    openRegKey(hKeyBase, baseKey, [&](HKEY hKey) {
        r = RegQueryValueEx(hKey, subKey, 0, nullptr, p, &bytes);
    });

    if(ERROR_SUCCESS != r) {
        _tcscpy_s(result.data(), result.size(), defaultVal);
    } else if(0 == bytes) {
        result[0] = 0;
    }
    return result.data();
}


#endif
