@echo off

"%ProgramFiles(x86)%\Zip\zip" -j \Temp\WindowsFrotz.zip Win\Release\Frotz.exe Win\Release\Frotz.chm
"%ProgramFiles(x86)%\Zip\zip" -j \Temp\WindowsFrotz.zip Win\Release\*.dll
"%ProgramFiles(x86)%\Zip\zip" \Temp\WindowsFrotz.zip COPYING Examples\*

"%ProgramFiles(x86)%\Zip\zip" \Temp\WindowsFrotzSrc.zip COPYING Blorb\*.c Blorb\*.h Generic\*.c Generic\*.h
"%ProgramFiles(x86)%\Zip\zip" \Temp\WindowsFrotzSrc.zip Win\* Win\res\* Win\help\* MakeDist.bat Installer\*
"%ProgramFiles(x86)%\Zip\zip" -r \Temp\WindowsFrotzSrc.zip Win\Frotz*\*
pushd \Programs
"%ProgramFiles(x86)%\Zip\zip" \Temp\WindowsFrotzSrc.zip Libraries\libmodplug\*
"%ProgramFiles(x86)%\Zip\zip" \Temp\WindowsFrotzSrc.zip Libraries\mfc\*
"%ProgramFiles(x86)%\Zip\zip" \Temp\WindowsFrotzSrc.zip Libraries\ScaleGfx\*.h
popd

pushd Installer
"%ProgramFiles(x86)%\NSIS\makensis" WindowsFrotz.nsi
move *.exe \Temp
popd
