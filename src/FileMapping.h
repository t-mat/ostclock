#ifndef FILE_MAPPING_H
#define FILE_MAPPING_H

struct FileMapping {
    FileMapping(const TCHAR* name, size_t bytes) {
        open(name, bytes);
    }

    ~FileMapping() {
        close();
    }

    void open(const TCHAR* name, size_t bytes) {
        HANDLE h = CreateFileMapping(
              INVALID_HANDLE_VALUE
            , nullptr
            , PAGE_READWRITE
            , 0
            , static_cast<DWORD>(bytes)
            , name
        );
        const auto cfmError = GetLastError();
        if(h) {
            if(cfmError == ERROR_ALREADY_EXISTS) {
                CloseHandle(h);
            } else {
                hMapObject = h;
                void* p = MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, 0);
                if(p) {
                    pMapPtr = p;
                    memset(p, 0, bytes);
                }
            }
        }
    }

    void close() {
        if(pMapPtr) {
            UnmapViewOfFile(pMapPtr);
            pMapPtr = nullptr;
        }

        if(hMapObject) {
            CloseHandle(hMapObject);
            hMapObject = nullptr;
        }
    }

    static bool access(const TCHAR* name, size_t bytes, const std::function<void(void*, size_t)>& func) {
        bool result = false;

        HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name);
        if(hMap) {
            void* pMem = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            if(pMem) {
                func(pMem, bytes);
                UnmapViewOfFile(pMem);
                result = true;
            }
            CloseHandle(hMap);
        }

        return result;
    }

protected:
    HANDLE hMapObject { nullptr };
    void* pMapPtr { nullptr };
};


#endif
