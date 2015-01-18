@if exist *.opensdf (
   @echo Abort : Visual Studio is running.
   @goto :EOF
)
@del /s *.idb *.pdb *.tlog *.log *.pch *.ipch *.cache *.lastbuildstate *.obj *.o *.map *.sdf *.suo *.ilk >nul 2>&1
@del /s *.exe *.dll *.lib *.exp >nul 2>&1
@del /s /A:AH *.idb *.pdb *.tlog *.log *.pch *.ipch *.cache *.lastbuildstate *.obj *.o *.map *.sdf *.suo *.ilk >nul 2>&1
@for /R . %%i in (ipch) do @rmdir /s /q "%%i" >nul 2>&1
@for /R . %%i in (ostclock) do @rmdir /s /q "%%i" >nul 2>&1
pushd dll
@for /R . %%i in (ostclock_dll) do @rmdir /s /q "%%i" >nul 2>&1
popd
