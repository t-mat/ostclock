#ifndef ENSURE_SINGLE_INSTANCE_H
#define ENSURE_SINGLE_INSTANCE_H

// Avoiding multiple instances of an application
//
//  int WinMain(...) {
//      const wchar_t* uniqueMutexName = L"Local\\MY-APP-NAME";
//      EnsureSingleInstance esi { uniqueMutexName };
//      if(! esi.good()) {
//          return 1; // exit process
//      }
//      ...
//  }
//
class EnsureSingleInstance {
public:
    EnsureSingleInstance() {}

    EnsureSingleInstance(const char* appMutexName) {
        open(appMutexName);
    }

    EnsureSingleInstance(const wchar_t* appMutexName) {
        open(appMutexName);
    }

    // Disallow copy
    EnsureSingleInstance(const EnsureSingleInstance&) = delete;
    EnsureSingleInstance& operator=(const EnsureSingleInstance&) = delete;

    // Allow move
    EnsureSingleInstance(EnsureSingleInstance&& x) {
        x.swap(*this);
    }

    EnsureSingleInstance& operator=(EnsureSingleInstance&& x) {
        x.swap(*this);
        return *this;
    }

    ~EnsureSingleInstance() {
        close();
    }

    void swap(EnsureSingleInstance& rhs) {
        using std::swap;
        swap(mutex, rhs.mutex);
        swap(status, rhs.status);
    }

    bool open(const char* appMutexName) {
        return open1(createMutex(nullptr, FALSE, appMutexName));
    }

    bool open(const wchar_t* appMutexName) {
        return open1(createMutex(nullptr, FALSE, appMutexName));
    }

    void close() {
        if(mutex) {
            ReleaseMutex(mutex);
            CloseHandle(mutex);
            mutex = nullptr;
        }
        status = false;
    }

    bool good() const {
        return status;
    }

    operator bool() const {
        return good();
    }

protected:
    struct Pair {
        HANDLE handle;
        DWORD error;
    };

    static Pair createMutex(LPSECURITY_ATTRIBUTES sa, BOOL io, const char* mn) {
        Pair m {};
        m.handle = CreateMutexA(sa, io, mn);
        m.error = GetLastError();
        return m;
    }

    static Pair createMutex(LPSECURITY_ATTRIBUTES sa, BOOL io, const wchar_t* mn) {
        Pair m {};
        m.handle = CreateMutexW(sa, io, mn);
        m.error = GetLastError();
        return m;
    }

    bool open1(Pair m) {
        if(!mutex) {
            mutex = m.handle;
            status = m.error != ERROR_ALREADY_EXISTS && m.error != ERROR_ACCESS_DENIED;
        }
        return good();
    }

private:
    HANDLE mutex { nullptr };
    bool status { false };
};

#endif
