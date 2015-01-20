@echo off & setlocal & setlocal ENABLEDELAYEDEXPANSION
pushd "%~dp0"

if exist *.opensdf (
  echo Abort : Visual Studio is running. Please close your project file.
  goto :EOF
)

set TARGET=%1

set SLN=ostclock.sln
set DEFAULT_TARGET=Rebuild

if "%TARGET%"=="" (
  set TARGET=%DEFAULT_TARGET%
  set NO_DEBUG=1
)


set LOGPARAMS=
set CLP=Verbosity=minimal;
set FLP=Detailed;Encoding=UTF-8

if "%TARGET%"=="Clean" (
  set LOGPARAMS=/verbosity:quiet
) else (
  set LOGPARAMS=/consoleloggerparameters:%CLP% /fl /fileloggerparameters:%FLP%
)

call "%VS120COMNTOOLS%\vsvars32.bat"

set PARAMS=/nologo /m %LOGPARAMS% /t:%TARGET% /p:Platform=x64

msbuild %SLN% %PARAMS% /p:Configuration=Release

if not "%NO_DEBUG%"=="1" (
  msbuild %SLN% %PARAMS% /p:Configuration=Debug
)

popd
