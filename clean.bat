@echo off & setlocal & setlocal ENABLEDELAYEDEXPANSION
pushd "%~dp0"
pushd msvc\vc2013
call clean.bat
popd
popd
