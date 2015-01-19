#include "stdafx.h"
#include "Registry.h"
#include "Config.h"

namespace {

const TCHAR myKey[] = _T(APP_REGKEY);

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


template<typename T = int, typename U = int>
T getRegLong(const TCHAR* subKey, U defaultVal = (U)0) {
    return getRegDword<T>(HKEY_CURRENT_USER, myKey, subKey, defaultVal);
};


std::wstring getRegStr(const TCHAR* subKey, const TCHAR* defaultVal = _T("")) {
    return getRegStr(HKEY_CURRENT_USER, myKey, subKey, defaultVal);
};


template<typename T = int>
void setRegLong(const TCHAR* subKey, T val) {
    setRegDword(HKEY_CURRENT_USER, myKey, subKey, val);
};


void setRegStr(const TCHAR* subKey, const TCHAR* val) {
    setRegStr(HKEY_CURRENT_USER, myKey, subKey, val);
};


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
              HKEY_CURRENT_USER
            , _T("Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager")
            , _T("ThemeActive")
            , _T("0")
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

//	dump(config);

    return config;
}


void saveConfig(const Config& config) {
//	dump(config);

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
