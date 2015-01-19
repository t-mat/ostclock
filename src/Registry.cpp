#include "stdafx.h"
#include "Registry.h"

namespace {

using RegFunc = std::function<void(HKEY)>;
const TCHAR myKey[] = _T(APP_REGKEY);


void openKey(const RegFunc& regFunc, const TCHAR* baseSubKey = myKey) {
    HKEY hKey = nullptr;
    RegOpenKey(HKEY_CURRENT_USER, baseSubKey, &hKey);
    if(hKey) {
        regFunc(hKey);
        RegCloseKey(hKey);
    }
}


void createKey(const RegFunc& regFunc, const TCHAR* baseSubKey = myKey) {
    HKEY hKey = nullptr;
    RegCreateKey(HKEY_CURRENT_USER, baseSubKey, &hKey);
    if(hKey) {
        regFunc(hKey);
        RegCloseKey(hKey);
    }
}


bool setRegLong(const TCHAR* entry, DWORD val) {
    LONG r = ERROR_INVALID_FUNCTION;
    const auto* p = reinterpret_cast<const BYTE*>(&val);
    const auto bytes = static_cast<DWORD>(sizeof(val));
    createKey([&](HKEY hKey) {
        r = RegSetValueEx(hKey, entry, 0, REG_DWORD, p, bytes);
    });
    return ERROR_SUCCESS == r;
}


bool setRegStr(const TCHAR* entry, const TCHAR* val) {
    LONG r = ERROR_INVALID_FUNCTION;
    const auto* p = reinterpret_cast<const BYTE*>(val);
    const auto bytes = static_cast<DWORD>(_tcslen(val) * sizeof(*val));
    createKey([&](HKEY hKey) {
        r = RegSetValueEx(hKey, entry, 0, REG_SZ, p, bytes);
    });
    return ERROR_SUCCESS == r;
}


LONG getRegLong(const TCHAR* entry, LONG defaultVal = 0) {
    LONG result = 0;
    LONG r = ERROR_INVALID_FUNCTION;
    DWORD size = sizeof(result);
    openKey([&](HKEY hKey) {
        r = RegQueryValueEx(hKey, entry, 0, nullptr, (LPBYTE)&result, &size);
    });
    if(ERROR_SUCCESS != r || sizeof(result) != size) {
        result = defaultVal;
    }
    return result;
}


template <class T, class U = LONG>
T getRegLong(const TCHAR* entry, U defval = (U)0) {
    return static_cast<T>(getRegLong(entry, (LONG) defval));
}


std::wstring getRegStr(
      const TCHAR* entry
    , const TCHAR* defval = _T("")
    , const TCHAR* baseSubKey = myKey
) {
    std::array<TCHAR, 1024> buf;

    DWORD size = static_cast<DWORD>(buf.size() * sizeof(buf[0]));
    LONG r = ERROR_INVALID_FUNCTION;

    openKey([&](HKEY hKey) {
        r = RegQueryValueEx(hKey, entry, 0, nullptr, (LPBYTE)buf.data(), &size);
    }, baseSubKey);

    if(ERROR_SUCCESS == r) {
        if(size == 0) {
            buf[0] = 0;
        }
    } else {
        _tcscpy_s(buf.data(), buf.size(), defval);
    }
    return buf.data();
}


void dump(const Config& config) {
    config;
    outputDebugString(L"config.text.width            = %d\n",       config.text.width           );
    outputDebugString(L"config.text.height           = %d\n",       config.text.height          );
    outputDebugString(L"config.text.xpos             = %d\n",       config.text.xPos            );
    outputDebugString(L"config.text.ypos             = %d\n",       config.text.yPos            );
    outputDebugString(L"config.text.lineSpacing      = %d\n",       config.text.lineSpacing     );
    outputDebugString(L"config.text.color.foreground = 0x%08x\n",   config.text.color.foreground);
    outputDebugString(L"config.text.color.background = 0x%08x\n",   config.text.color.background);
    outputDebugString(L"config.text.format           = [%s]\n",     config.text.format.c_str()  );
    outputDebugString(L"config.font.name             = [%s]\n",     config.font.name.c_str()    );
    outputDebugString(L"config.font.size             = %d\n",       config.font.size            );
    outputDebugString(L"config.font.italic           = %d\n",       config.font.italic          );
    outputDebugString(L"config.font.bold             = %d\n",       config.font.bold            );
    outputDebugString(L"config.font.langId           = %d\n",       config.font.langId          );
    outputDebugString(L"config.font.quality          = %d\n",       config.font.quality         );
}

} // Anonymous namespace


Config loadConfig() {
    Config config;
    config.text.width               = getRegLong(_T("Width"));
    config.text.height              = getRegLong(_T("Height"));
    config.text.xPos                = getRegLong(_T("Xpos"));
    config.text.yPos                = getRegLong(_T("Ypos"));
    config.text.lineSpacing         = getRegLong(_T("LineSpacing"));
    config.text.color.foreground    = getRegLong(_T("ForegroundColor"), GetSysColor(COLOR_BTNTEXT));
    config.text.color.background    = getRegLong(_T("BackgroundColor"), GetSysColor(COLOR_3DFACE));
    config.text.format              = getRegStr (_T("Format"), _T("%H:%M:%S"));
    config.font.name                = getRegStr (_T("FontName"));
    config.font.size                = getRegLong(_T("FontSize"), 10);
    config.font.italic              = getRegLong(_T("FontItalic"));
    config.font.bold                = getRegLong(_T("FontBold"), 1);
    config.font.langId              = getRegLong(_T("FontLocale"), GetUserDefaultLangID());
    config.font.quality             = getRegLong(_T("FontQuality"), ANTIALIASED_QUALITY);

    {
        const auto t = getRegStr(
              _T("ThemeActive")
            , _T("0")
            , _T("Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager")
        );
        config.explore.isXpStyle = (_T('1') == t[0]);
    }

    if(config.font.name[0] == 0) {
        if(auto hFont = GetStockObject(DEFAULT_GUI_FONT)) {
            auto lf = getObject<LOGFONT>(hFont);
            config.font.name = lf.lfFaceName;
        }
    }

    config.font.size = std::max<int>(6, config.font.size);

    dump(config);

    return config;
}


void saveConfig(const Config& config) {
    dump(config);

    setRegLong(_T("Width")              , config.text.width);
    setRegLong(_T("Height")             , config.text.height);
    setRegLong(_T("Xpos")               , config.text.xPos);
    setRegLong(_T("Ypos")               , config.text.yPos);
    setRegLong(_T("LineSpacing")        , config.text.lineSpacing);
    setRegLong(_T("ForegroundColor")    , config.text.color.foreground);
    setRegLong(_T("BackgroundColor")    , config.text.color.background);
    setRegStr (_T("Format")             , config.text.format.c_str());
    setRegStr (_T("FontName")           , config.font.name.c_str());
    setRegLong(_T("FontSize")           , config.font.size);
    setRegLong(_T("FontItalic")         , config.font.italic);
    setRegLong(_T("FontBold")           , config.font.bold);
    setRegLong(_T("FontLocale")         , config.font.langId);
    setRegLong(_T("FontQuality")        , config.font.quality);
}
