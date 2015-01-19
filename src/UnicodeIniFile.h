#ifndef UNICODE_INI_FILE_H
#define UNICODE_INI_FILE_H

#include <shlwapi.h> // PathRemoveFileSpecW
#include <shlobj.h>  // CSIDL_*, SHGetFolderPathW
#include <lmcons.h>	 // UNLEN
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#include <map>
#include <string>
#include <vector>

#pragma comment(lib, "shlwapi.lib")

// Using Unicode in INI files
// http://www.codeproject.com/Articles/9071/
class UnicodeIniFile {
public:
    using String = std::wstring;

    enum Type {
        //	Module name : C:\path\to\app\MyApp.exe
        TYPE_EXE = 0,		// C:\path\to\app\MyApp.ini
        TYPE_EXE_PER_USER,	// C:\path\to\app\%USERNAME%\MyApp.ini
        TYPE_APPDATA		// %APPDATA%\MyApp\MyApp.ini
    };

    UnicodeIniFile(
          Type type = TYPE_APPDATA
        , const wchar_t* prefix = L""
        , const wchar_t* suffix = L".ini"
    )
        : type { type }
        , prefix (prefix)
        , suffix (suffix)
    {}

    ~UnicodeIniFile() {}

    void load() {
        return load(makeFilename(type, prefix.c_str(), suffix.c_str()));
    }

    void load(const String& filename) {
        sectionNames.clear();
        sectionKeyNames.clear();
        properties.clear();

        // load section names
        {
            std::vector<wchar_t> secBuf(65536);
            const auto sectionsLen = ::GetPrivateProfileSectionNamesW(
                  secBuf.data()
                , static_cast<DWORD>(secBuf.size())
                , filename.c_str()
            );
            if(sectionsLen > 0) {
                secBuf.resize(sectionsLen);
                secBuf.push_back(0);
                const auto* p = secBuf.data();
                const auto* e = &secBuf.back();
                while(p < e && *p != 0) {
                    const auto len = wcslen(p);
                    sectionNames.push_back(p);
                    p += len + 1;
                }
            }
        }

        // load key names
        for(const auto& sectionName : sectionNames) {
            std::vector<wchar_t> kvBuf(65536);
            const auto namesLen = ::GetPrivateProfileSectionW(
                  sectionName.c_str()
                , kvBuf.data()
                , static_cast<DWORD>(kvBuf.size())
                , filename.c_str()
            );
            if(namesLen > 0) {
                auto& section = sectionKeyNames[sectionName];
                kvBuf.resize(namesLen);
                kvBuf.push_back(0);
                auto* p = kvBuf.data();
                auto* e = &kvBuf.back();
                while(p < e && *p != 0) {
                    const auto len = wcslen(p);
                    for(auto* q = p; *q != 0; ++q) {
                        if(*q == L'=') {
                            *q = 0;
                            break;
                        }
                    }
                    section.push_back(p);
                    p += len + 1;
                }
            }
        }

        // load values
        for(const auto& sectionName : sectionNames) {
            const auto& keyNames = sectionKeyNames[sectionName];
            for(const auto& keyName : keyNames) {
                std::vector<wchar_t> valBuf(65536);
                const auto len = ::GetPrivateProfileStringW(
                      sectionName.c_str()
                    , keyName.c_str()
                    , L""
                    , valBuf.data()
                    , static_cast<DWORD>(valBuf.size())
                    , filename.c_str()
                );
                valBuf[len] = 0;
                properties[sectionName][keyName] = valBuf.data();
            }
        }
    }

    void save() {
        return save(makeFilename(type, prefix.c_str(), suffix.c_str()));
    }

    void save(const String& filename) const {
        setupDirectory(filename.c_str());

        if(::GetFileAttributesW(filename.c_str()) == 0xffffffff) {
            const wchar_t* defaultSectionName = L"Settings";
            auto h = ::CreateFileW(filename.c_str(), GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
            if(h != INVALID_HANDLE_VALUE) {
                const WORD bom = 0xfeff;
                DWORD dw {};
                wchar_t buf[256] {};
                const auto len = _snwprintf_s(buf, _countof(buf), L"[%s]", defaultSectionName);
                ::WriteFile(h, &bom, sizeof(bom), &dw, nullptr);
                ::WriteFile(h, buf, sizeof(buf[0]) * (len+1), &dw, nullptr);
                ::CloseHandle(h);
            }
        }

        for(const auto& sectionName : sectionNames) {
            const auto& keyNames = sectionKeyNames.at(sectionName);
            for(const auto& keyName : keyNames) {
                const auto& val = properties[sectionName][keyName];
                ::WritePrivateProfileStringW(
                      sectionName.c_str()
                    , keyName.c_str()
                    , val.c_str()
                    , filename.c_str()
                );
            }
        }
    }

    bool weakSet(const String& section, const String& key, const String& val) const {
        const auto it = properties.find(section);
        if(it == properties.end()) {
            sectionNames.push_back(section);
            sectionKeyNames[section].push_back(key);
            properties[section][key] = val;
            return true;
        }

        const auto it2 = it->second.find(key);
        if(it2 == it->second.end()) {
            sectionKeyNames[section].push_back(key);
            properties[section][key] = val;
            return true;
        }

        return false;
    }

    const String get(const String& section, const String& key, const String& defaultVal) const {
        if(weakSet(section, key, defaultVal)) {
            return defaultVal;
        }
        return properties[section][key];
    }

    void set(const String& section, const String& key, const String& val) {
        if(weakSet(section, key, val)) {
            return;
        }
        properties[section][key] = val;
    }

    const String getString(const String& section, const String& key, const String& defaultVal = String{}) const {
        return get(section, key, defaultVal);
    }

    void setString(const String& section, const String& key, const String& val) {
        return set(section, key, val);
    }

    int getInt(const String& section, const String& key, int defaultVal = 0) const {
        return std::stoi(get(section, key, std::to_wstring(defaultVal)));
    }

    void setInt(const String& section, const String& key, int val) {
        return set(section, key, std::to_wstring(val));
    }

    static String getUsername() {
        wchar_t username[UNLEN+1] {};
        DWORD usernameSize = _countof(username);
        const auto result = ::GetUserName(username, &usernameSize);
        if(0 == result) {
            username[0] = 0;
        }
        return username;
    }

    static String getAppDataDir() {
        wchar_t appDataPath[MAX_PATH+1] {};
        const int folder = CSIDL_APPDATA | CSIDL_FLAG_CREATE; // %APPDATA%
        if(FAILED(::SHGetFolderPathW(NULL, folder, NULL, 0, appDataPath))) {
            appDataPath[0] = 0;
        }
        return appDataPath;
    }

    static String makeFilename(Type type = TYPE_APPDATA, const wchar_t* prefix = L"", const wchar_t* suffix = L".ini") {
        wchar_t modFilename[MAX_PATH+1] {};
        ::GetModuleFileNameW(nullptr, modFilename, _countof(modFilename));

        wchar_t modDrive[_MAX_DRIVE+1] {};
        wchar_t modDir[_MAX_DIR+1] {};
        wchar_t modFname[_MAX_FNAME+1] {};
        wchar_t modExt[_MAX_EXT+1] {};
        _wsplitpath_s(modFilename, modDrive, modDir, modFname, modExt);

        wchar_t filename[MAX_PATH+1] {};

        switch(type) {
        default:
        case TYPE_APPDATA:
            {
                const auto appData = getAppDataDir();
                if(prefix[0]) {
                    _snwprintf_s(
                        filename, _countof(filename)
                        , L"%s\\%s\\%s\\%s%s"
                        , appData.c_str()
                        , prefix
                        , modFname
                        , modFname
                        , suffix
                    );
                } else {
                    _snwprintf_s(
                        filename, _countof(filename)
                        , L"%s\\%s\\%s%s"
                        , appData.c_str()
                        , modFname
                        , modFname
                        , suffix
                    );
                }
            }
            break;

        case TYPE_EXE:
        case TYPE_EXE_PER_USER:
            {
                wchar_t fileDir[MAX_PATH+1] {};
                if(TYPE_EXE == type) {
                    _snwprintf_s(fileDir, _countof(fileDir), L"%s%s", modDrive, modDir);
                } else {
                    const auto username = getUsername();
                    _snwprintf_s(fileDir, _countof(fileDir), L"%s%s%s\\", modDrive, modDir, username.c_str());
                }
                _snwprintf_s(filename, _countof(filename), L"%s%s%s%s", fileDir, prefix, modFname, suffix);
            }
            break;
        }

        return filename;
    }

    static void setupDirectory(const wchar_t* filename) {
        wchar_t path[MAX_PATH+1] = { 0 };
        ::wcscpy_s(path, _countof(path), filename);
        ::PathRemoveFileSpecW(path);
        if(::wcscmp(path, filename) != 0) {
            if(::GetFileAttributesW(path) == 0xffffffff) {
                setupDirectory(path); // recursive
                ::CreateDirectoryW(path, NULL);
            }
        }
    }

    Type	type { TYPE_APPDATA };
    String	prefix {};
    String	suffix {};
    mutable std::vector<String>							sectionNames;
    mutable std::map<String, std::vector<String>>		sectionKeyNames;
    mutable std::map<String, std::map<String, String>>	properties;
};

#endif
