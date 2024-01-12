@echo off
rmdir /s /q Win\Debug
rmdir /s /q Win\Release\Frotz.tlog
del Win\Release\*.lib
del Win\Release\*.log
del Win\Release\*.obj
del Win\Release\*.pch
del Win\Release\*.recipe
del Win\Release\*.res
del Win\Release\*.txt
rmdir /s /q Win\FrotzDeutsch\Release
rmdir /s /q Win\FrotzEspa¤ol\Release
rmdir /s /q Win\FrotzFran‡ais\Release
rmdir /s /q Win\FrotzItaliano\Release
rmdir /s /q Win\FrotzRussian\Release
del /s Win\*.user

