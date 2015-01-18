@echo off & setlocal

set SLN=ostclock.sln
call "%VS120COMNTOOLS%\vsvars32.bat"

msbuild %SLN% /nologo /m /t:Rebuild /p:Platform=x64 /p:Configuration=Release
msbuild %SLN% /nologo /m /t:Rebuild /p:Platform=x64 /p:Configuration=Debug
