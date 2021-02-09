; Install script for Windows Frotz

!include "MUI2.nsh"
!include "WindowsFrotz.nsh"

Name "Windows Frotz"
Caption "Windows Frotz ${FROTZ_VERSION} Setup"
BrandingText "NullSoft Install System"
Unicode true
ManifestDPIAware true

SetCompressor /SOLID lzma
RequestExecutionLevel admin
OutFile "WindowsFrotzInstaller.exe"

InstallDir "$PROGRAMFILES\Windows Frotz"
InstallDirRegKey HKLM "SOFTWARE\David Kinder\Frotz\Install" "Directory"

!define MUI_ICON "..\Win\res\Infocom.ico"
!define MUI_UNICON "..\Win\res\Infocom.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "Back.bmp"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN $INSTDIR\Frotz.exe
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "DoInstall"

  SetOutPath "$INSTDIR"
  File "..\Win\Release\Frotz.exe"
  File "..\Win\Release\FrotzDeutsch.dll"
  File "..\Win\Release\FrotzEspañol.dll"
  File "..\Win\Release\FrotzFrançais.dll"
  File "..\Win\Release\FrotzItaliano.dll"
  File "..\Win\Release\FrotzRussian.dll"
  File "..\Win\Release\Frotz.chm"
  File "..\COPYING"
  SetOutPath "$INSTDIR\Examples"
  File "..\Examples\Unicode.inf"
  File "..\Examples\Unicode.z5"
  File "..\Examples\ZSpec11.inf"
  File "..\Examples\ZSpec11.z6"
  WriteUninstaller "Uninstall.exe"

  ; Registry keys for the IFDB Download Advisor
  WriteRegStr HKLM "Software\IFDB.tads.org\MetaInstaller\Interpreters\WindowsFrotz.DavidKinder" "Version" ${FROTZ_VERSION}
  WriteRegStr HKLM "Software\IFDB.tads.org\MetaInstaller\Interpreters\WindowsFrotz.DavidKinder" "RunGame" '"$INSTDIR\Frotz.exe" "%1"'

  SetShellVarContext all    
  ; Remove old Start Menu folder
  RMDir /r "$SMPROGRAMS\Windows Frotz"
  CreateShortCut "$SMPROGRAMS\Windows Frotz.lnk" "$INSTDIR\Frotz.exe"
  SetShellVarContext current
  
  WriteRegStr HKLM "SOFTWARE\David Kinder\Frotz\Install" "Directory" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsFrotz" "DisplayName" "Windows Frotz"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsFrotz" "DisplayIcon" "$INSTDIR\Frotz.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsFrotz" "DisplayVersion" ${FROTZ_VERSION}
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsFrotz" "Publisher" "David Kinder"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsFrotz" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsFrotz" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsFrotz" "NoRepair" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsFrotz" "EstimatedSize" 1665


  WriteRegStr HKCR ".z1" "" "ZMachine.V1"
  WriteRegStr HKCR "ZMachine.V1" "" "Z-code V1 Adventure"
  WriteRegStr HKCR "ZMachine.V1\DefaultIcon" "" "$INSTDIR\Frotz.exe,1"
  WriteRegStr HKCR "ZMachine.V1\shell" "" "open"
  WriteRegStr HKCR "ZMachine.V1\shell\open\command" "" '"$INSTDIR\Frotz.exe" "%1"'

  WriteRegStr HKCR ".z2" "" "ZMachine.V2"
  WriteRegStr HKCR "ZMachine.V2" "" "Z-code V2 Adventure"
  WriteRegStr HKCR "ZMachine.V2\DefaultIcon" "" "$INSTDIR\Frotz.exe,2"
  WriteRegStr HKCR "ZMachine.V2\shell" "" "open"
  WriteRegStr HKCR "ZMachine.V2\shell\open\command" "" '"$INSTDIR\Frotz.exe" "%1"'

  WriteRegStr HKCR ".z3" "" "ZMachine.V3"
  WriteRegStr HKCR "ZMachine.V3" "" "Z-code V3 Adventure"
  WriteRegStr HKCR "ZMachine.V3\DefaultIcon" "" "$INSTDIR\Frotz.exe,3"
  WriteRegStr HKCR "ZMachine.V3\shell" "" "open"
  WriteRegStr HKCR "ZMachine.V3\shell\open\command" "" '"$INSTDIR\Frotz.exe" "%1"'

  WriteRegStr HKCR ".z4" "" "ZMachine.V4"
  WriteRegStr HKCR "ZMachine.V4" "" "Z-code V4 Adventure"
  WriteRegStr HKCR "ZMachine.V4\DefaultIcon" "" "$INSTDIR\Frotz.exe,4"
  WriteRegStr HKCR "ZMachine.V4\shell" "" "open"
  WriteRegStr HKCR "ZMachine.V4\shell\open\command" "" '"$INSTDIR\Frotz.exe" "%1"'

  WriteRegStr HKCR ".z5" "" "ZMachine.V5"
  WriteRegStr HKCR "ZMachine.V5" "" "Z-code V5 Adventure"
  WriteRegStr HKCR "ZMachine.V5\DefaultIcon" "" "$INSTDIR\Frotz.exe,5"
  WriteRegStr HKCR "ZMachine.V5\shell" "" "open"
  WriteRegStr HKCR "ZMachine.V5\shell\open\command" "" '"$INSTDIR\Frotz.exe" "%1"'

  WriteRegStr HKCR ".z6" "" "ZMachine.V6"
  WriteRegStr HKCR "ZMachine.V6" "" "Z-code V6 Adventure"
  WriteRegStr HKCR "ZMachine.V6\DefaultIcon" "" "$INSTDIR\Frotz.exe,6"
  WriteRegStr HKCR "ZMachine.V6\shell" "" "open"
  WriteRegStr HKCR "ZMachine.V6\shell\open\command" "" '"$INSTDIR\Frotz.exe" "%1"'

  WriteRegStr HKCR ".z7" "" "ZMachine.V7"
  WriteRegStr HKCR "ZMachine.V7" "" "Z-code V7 Adventure"
  WriteRegStr HKCR "ZMachine.V7\DefaultIcon" "" "$INSTDIR\Frotz.exe,7"
  WriteRegStr HKCR "ZMachine.V7\shell" "" "open"
  WriteRegStr HKCR "ZMachine.V7\shell\open\command" "" '"$INSTDIR\Frotz.exe" "%1"'

  WriteRegStr HKCR ".z8" "" "ZMachine.V8"
  WriteRegStr HKCR "ZMachine.V8" "" "Z-code V8 Adventure"
  WriteRegStr HKCR "ZMachine.V8\DefaultIcon" "" "$INSTDIR\Frotz.exe,8"
  WriteRegStr HKCR "ZMachine.V8\shell" "" "open"
  WriteRegStr HKCR "ZMachine.V8\shell\open\command" "" '"$INSTDIR\Frotz.exe" "%1"'

  WriteRegStr HKCR ".zlb" "" "ZMachine.Blorb"
  WriteRegStr HKCR ".zblorb" "" "ZMachine.Blorb"
  WriteRegStr HKCR "ZMachine.Blorb\DefaultIcon" "" "$INSTDIR\Frotz.exe,9"
  WriteRegStr HKCR "ZMachine.Blorb" "" "Blorbed Z-code Adventure"
  WriteRegStr HKCR "ZMachine.Blorb\shell" "" "open"
  WriteRegStr HKCR "ZMachine.Blorb\shell\open\command" "" '"$INSTDIR\Frotz.exe" "%1"'

SectionEnd

Section "Uninstall"

  RMDir /r "$INSTDIR\Examples"
  Delete "$INSTDIR\Frotz.exe"
  Delete "$INSTDIR\FrotzDeutsch.dll"
  Delete "$INSTDIR\FrotzEspañol.dll"
  Delete "$INSTDIR\FrotzFrançais.dll"
  Delete "$INSTDIR\FrotzItaliano.dll"
  Delete "$INSTDIR\FrotzRussian.dll"
  Delete "$INSTDIR\Frotz.chm"
  Delete "$INSTDIR\COPYING"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"

  SetShellVarContext all
  Delete "$SMPROGRAMS\Windows Frotz.lnk"
  SetShellVarContext current

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsFrotz"
  DeleteRegKey HKLM "Software\IFDB.tads.org\MetaInstaller\Interpreters\WindowsFrotz.DavidKinder"

  DeleteRegKey HKCR ".z1"
  DeleteRegKey HKCR ".z2"
  DeleteRegKey HKCR ".z3"
  DeleteRegKey HKCR ".z4"
  DeleteRegKey HKCR ".z5"
  DeleteRegKey HKCR ".z6"
  DeleteRegKey HKCR ".z7"
  DeleteRegKey HKCR ".z8"
  DeleteRegKey HKCR ".zlb"
  DeleteRegKey HKCR ".zblorb"

  DeleteRegKey HKCR "ZMachine.V1"
  DeleteRegKey HKCR "ZMachine.V2"
  DeleteRegKey HKCR "ZMachine.V3"
  DeleteRegKey HKCR "ZMachine.V4"
  DeleteRegKey HKCR "ZMachine.V5"
  DeleteRegKey HKCR "ZMachine.V6"
  DeleteRegKey HKCR "ZMachine.V7"
  DeleteRegKey HKCR "ZMachine.V8"
  DeleteRegKey HKCR "ZMachine.Blorb"

SectionEnd
