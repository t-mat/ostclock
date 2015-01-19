#ifndef OSTCLOCK_H
#define OSTCLOCK_H

#define APPNAME                 "ostclock"
#define APP_REGKEY              "Software\\ostcllock\\ostclock"
#define APP_SHARED_MEMORY_NAME  _T(APPNAME) _T("SharedMemory")
#ifdef NDEBUG
#  define APP_DLL_NAME          "ostclock_dll.dll"
#  define OUTPUT_DEBUG_STRING_ENABLE
#else
#  define APP_DLL_NAME          "ostclock_dll_debug.dll"
#  define OUTPUT_DEBUG_STRING_ENABLE
#endif

#define NOMINMAX
#pragma warning(disable : 4351)
#pragma warning(disable : 4822)

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

#include <commctrl.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <process.h>
#include <psapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include <winver.h>
#include <wtsapi32.h>

#include <afxres.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "VisualCppBasicMacro.h"
#include "WindowsBasicApi.h"
#include "OutputDebugString.h"

#endif
