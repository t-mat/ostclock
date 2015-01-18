#ifndef DLL_LOADER_H
#define DLL_LOADER_H

class DllLoader {
public:
	DllLoader() {}

	DllLoader(const TCHAR* filename) {
		load(filename);
	}

	~DllLoader() {
		unload();
	}

	bool load(const TCHAR* filename) {
		if(!loadFailed && !good()) {
			hInstance = LoadLibrary(filename);
			loadFailed = !good();
		}
		return good();
	}

	void unload() {
		if(good()) {
			FreeLibrary(hInstance);
			hInstance = nullptr;
		}
	}

	bool good() const {
		return nullptr != hInstance;
	}

	bool isLoadFailed() const {
		return loadFailed;
	}

	template<typename T = void*>
	bool getProcAddress(T*& funcPtr, const char* procName) const {
		void* ptr = nullptr;
		if(good()) {
			ptr = GetProcAddress(hInstance, procName);
		}
		funcPtr = (T*) ptr;
		return ptr != nullptr;
	}

	HINSTANCE getHinstance() const {
		return hInstance;
	}

protected:
	bool loadFailed { false };
	HINSTANCE hInstance { nullptr };
};

#endif
