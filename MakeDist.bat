@echo off

"\Program Files\Zip\zip" -j \Temp\WindowsFrotz.zip Win\Release\Frotz.exe Win\Release\Frotz.chm
"\Program Files\Zip\zip" -j \Temp\WindowsFrotz.zip Win\Release\*.dll
"\Program Files\Zip\zip" \Temp\WindowsFrotz.zip COPYING Examples\*

"\Program Files\Zip\zip" \Temp\WindowsFrotzSrc.zip COPYING Blorb\*.c Blorb\*.h Generic\*.c Generic\*.h
"\Program Files\Zip\zip" \Temp\WindowsFrotzSrc.zip Win\* Win\res\* Win\help\* MakeDist.bat Installer\*
"\Program Files\Zip\zip" -r \Temp\WindowsFrotzSrc.zip Win\Frotz*\*
pushd \Programs
"\Program Files\Zip\zip" \Temp\WindowsFrotzSrc.zip Libraries\libmodplug\*
"\Program Files\Zip\zip" \Temp\WindowsFrotzSrc.zip Libraries\mfc\*
"\Program Files\Zip\zip" \Temp\WindowsFrotzSrc.zip Libraries\ScaleGfx\*.h
"\Program Files\Zip\zip" \Temp\WindowsFrotzSrc.zip Libraries\ScaleGfx\*.cpp
"\Program Files\Zip\zip" \Temp\WindowsFrotzSrc.zip Libraries\ScaleGfx\*.def
"\Program Files\Zip\zip" \Temp\WindowsFrotzSrc.zip Libraries\ScaleGfx\Makefile
popd

pushd Installer
"\Program Files\NSIS\makensis" WindowsFrotz.nsi
move *.exe \Temp
popd
