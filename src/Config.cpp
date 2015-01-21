#include "stdafx.h"
#include "Registry.h"
#include "Config.h"
#include "SharedVariable.h"
#include "UnicodeIniFile.h"

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


Config loadConfig1(
      const std::function<int (const TCHAR*, int)>& getInt
    , const std::function<std::wstring (const TCHAR*, const TCHAR*)>& getStr
) {
    Config config;

    config.text.width               = getInt(_T("Width"), 0);
    config.text.height              = getInt(_T("Height"), 0);
    config.text.xPos                = getInt(_T("Xpos"), 0);
    config.text.yPos                = getInt(_T("Ypos"), 0);
    config.text.lineSpacing         = getInt(_T("LineSpacing"), 0);
    config.text.color.foreground    = getInt(_T("ForegroundColor"), GetSysColor(COLOR_BTNTEXT));
    config.text.color.background    = getInt(_T("BackgroundColor"), GetSysColor(COLOR_3DFACE));
    config.text.format              = getStr(_T("Format"), _T("%H:%M:%S"));
    config.font.name                = getStr(_T("FontName"), _T(""));
    config.font.size                = getInt(_T("FontSize"), 10);
    config.font.italic              = getInt(_T("FontItalic"), 0);
    config.font.bold                = getInt(_T("FontBold"), 1);
    config.font.langId              = getInt(_T("FontLocale"), GetUserDefaultLangID());
    config.font.quality             = getInt(_T("FontQuality"), ANTIALIASED_QUALITY);

    return config;
}


void saveConfig1(
      const Config& config
    , const std::function<void(const TCHAR*, int)>& setInt
    , const std::function<void(const TCHAR*, const TCHAR*)>& setStr
) {
    setInt(_T("Width")              , config.text.width);
    setInt(_T("Height")             , config.text.height);
    setInt(_T("Xpos")               , config.text.xPos);
    setInt(_T("Ypos")               , config.text.yPos);
    setInt(_T("LineSpacing")        , config.text.lineSpacing);
    setInt(_T("ForegroundColor")    , config.text.color.foreground);
    setInt(_T("BackgroundColor")    , config.text.color.background);
    setStr(_T("Format")             , config.text.format.c_str());
    setStr(_T("FontName")           , config.font.name.c_str());
    setInt(_T("FontSize")           , config.font.size);
    setInt(_T("FontItalic")         , config.font.italic);
    setInt(_T("FontBold")           , config.font.bold);
    setInt(_T("FontLocale")         , config.font.langId);
    setInt(_T("FontQuality")        , config.font.quality);
}


struct IniFile {
    IniFile()
        : section(_T("Settings"))
    {
        if(accessSharedVariable([&](const SharedVariable& sv) {
            _tcscpy_s(iniFilename, sv.iniFilename);
        })) {
            if(iniFilename[0]) {
                uif.load(iniFilename);
            }
        }
    }

    IniFile(const IniFile&) = delete;
    IniFile& operator=(const IniFile&) = delete;

    void save() {
        uif.save(iniFilename);
    }

    UnicodeIniFile uif {};
    TCHAR iniFilename[MAX_PATH+1] {};
    const std::wstring section;
};

} // Anonymous namespace


Config loadConfig(bool useIniFile) {
    Config config;

    if(useIniFile) {
        IniFile iniFile {};

        config = loadConfig1(
              [&](const TCHAR* key, int defaultVal) {
                auto val = iniFile.uif.getInt(iniFile.section, key, defaultVal);
                return val;
            }
            , [&](const TCHAR* key, const TCHAR* defaultVal) {
                auto val = iniFile.uif.getString(iniFile.section, key, defaultVal);
                return val;
            }
        );
    } else {
        config = loadConfig1(
              [&](const TCHAR* key, int defaultVal) {
                auto val = getRegDword<int>(HKEY_CURRENT_USER, myKey, key, defaultVal);
                return val;
            }
            , [&](const TCHAR* key, const TCHAR* defaultVal) {
                auto val = getRegStr(HKEY_CURRENT_USER, myKey, key, defaultVal);
                return val;
            }
        );
    }

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


void saveConfig(const Config& config, bool useIniFile) {
//	dump(config);

    if(useIniFile) {
        IniFile iniFile {};

        saveConfig1(config
            , [&](const TCHAR* key, int val) {
                iniFile.uif.setInt(iniFile.section, key, val);
            }
            , [&](const TCHAR* key, const TCHAR* val) {
                iniFile.uif.setString(iniFile.section, key, val);
            }
        );

        iniFile.save();
    } else {
        saveConfig1(config
            , [&](const TCHAR* key, int val) {
                setRegDword(HKEY_CURRENT_USER, myKey, key, val);
            }
            , [&](const TCHAR* key, const TCHAR* val) {
                setRegStr(HKEY_CURRENT_USER, myKey, key, val);
            }
        );
    }
}
