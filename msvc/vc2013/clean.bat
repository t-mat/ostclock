@echo off & setlocal & setlocal ENABLEDELAYEDEXPANSION

@rem "Clean solution" does not delete .ipch files
@rem https://connect.microsoft.com/VisualStudio/feedback/details/798648/
@rem
@rem Escape asterisk in Windows Batch File's FOR Loop
@rem http://stackoverflow.com/questions/17071465/
@rem

pushd "%~dp0"

if exist *.opensdf (
  echo Abort : Visual Studio is running. Please close your project file.
  goto :EOF
)


call build.bat Clean


set DIRS=ipch build_tmp

for /R . %%i in (%DIRS%) do (
  @rmdir /s /q "%%i" >nul 2>&1
)


for %%i in (1 2 3) do (
  set D=
  if 1==%%i ( set D=del /q /s )
  if 2==%%i ( set D=del /q /s /A:A )
  if 3==%%i ( set D=del /q /s /A:AH )

  !D! *.cache             >nul 2>&1
  !D! *.dll               >nul 2>&1
  !D! *.exe               >nul 2>&1
  !D! *.exp               >nul 2>&1
  !D! *.idb               >nul 2>&1
  !D! *.ilk               >nul 2>&1
  !D! *.ini               >nul 2>&1
  !D! *.lib               >nul 2>&1
  !D! *.log               >nul 2>&1
  !D! *.ipch              >nul 2>&1
  !D! *.lastbuildstateobj >nul 2>&1
  !D! *.map               >nul 2>&1
  !D! *.o                 >nul 2>&1
  !D! *.pch               >nul 2>&1
  !D! *.pdb               >nul 2>&1
  !D! *.sdf               >nul 2>&1
  !D! *.suo               >nul 2>&1
  !D! *.tlog              >nul 2>&1
)

popd
