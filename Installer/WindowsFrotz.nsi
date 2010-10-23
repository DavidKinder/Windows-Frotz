; Installer for Windows Frotz

;--------------------------------
; Game bundling options
; Comment out the BUNDLE_GAME line to create regular Windows Frotz installer.
;
;!define BUNDLE_GAME
!define GAME_FILE "Vacation.z5"
!define GAME_NAME "Vacation Gone Awry"
!define GAME_SHORT_NAME "Vacation"
!define GAME_RELEASE " R3"
;--------------------------------

!define FROTZ_VERSION "1.16"

;--------------------------------
;Configuration

!include "MUI.nsh"

SetCompressor /SOLID lzma
RequestExecutionLevel admin

; The name of the installer and the file to write
!ifdef BUNDLE_GAME
  Name "${GAME_NAME} and Windows Frotz"
  Caption "${GAME_NAME}"
  BrandingText "NullSoft Install System"
  OutFile "${GAME_SHORT_NAME}AndWindowsFrotz.exe"
!else
  Name "Windows Frotz"
  Caption "Windows Frotz ${FROTZ_VERSION} Setup"
  BrandingText "NullSoft Install System"
  OutFile "WindowsFrotzInstaller.exe"
!endif

; The default installation directory
InstallDir "$PROGRAMFILES\Windows Frotz"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\WindowsFrotz "Install_Dir"

!define MUI_ICON "..\Win\res\Infocom.ico"
!define MUI_UNICON "..\Win\res\Infocom.ico"

!define MUI_HEADERIMAGE

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY

Var STARTMENU_FOLDER
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Windows Frotz"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
!define MUI_STARTMENUPAGE_REGISTRY_KEY SOFTWARE\WindowsFrotz
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME Startmenu_Folder_Name
!insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER

Page custom chooseAssoc

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN $INSTDIR\Frotz.exe
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Function .onInit
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "WindowsFrotzMutex") i .r1 ?e'
  Pop $R0
 
  StrCmp $R0 0 +3
    MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running."
    Abort

  InitPluginsDir
  File "/oname=$PLUGINSDIR\WindowsFrotz.ini" WindowsFrotz.ini
FunctionEnd

Var UserType
Var InstallFor

Function getUserType
	ClearErrors
	UserInfo::GetName
	IfErrors Win9x
	Pop $0
	UserInfo::GetAccountType
	Pop $UserType
        Return
	
	Win9x:
		# This one means you don't need to care about admin or
		# not admin because Windows 9x doesn't either
	   StrCpy $UserType "Admin"
FunctionEnd

Function chooseAssoc
  Push $R0
  !insertmacro MUI_HEADER_TEXT "Choose Options" "Choose how Windows Frotz is configured."
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY_RETURN WindowsFrotz.ini
  Pop $R0

  StrCmp $R0 "success" +1 leaveThis
  
  ReadINIStr $R0 "$PLUGINSDIR\WindowsFrotz.ini" "Field 5" "State" ; Reads the Install for ALL option state

  StrCpy $InstallFor "current"
  StrCmp $R0 "0" leaveThis
    Call getUserType
    StrCmp $UserType "Admin" +1 notAdmin
      SetShellVarContext all
      StrCpy $InstallFor "all"
      goto leaveThis
notAdmin:
    MessageBox MB_OK "You do not have administrator rights. A single user install will be performed."

leaveThis:

FunctionEnd


;--------------------------------

; The stuff to install
Section "MainInstall" ;No components page, name is not important

  SectionIn RO

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\WindowsFrotz "Install_Dir" "$INSTDIR"

  ; Write the install type into the registry
  WriteRegStr HKLM SOFTWARE\WindowsFrotz "InstallFor" "$InstallFor"

  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR"
  
  ; Put files there
  File "..\Win\Release\Frotz.exe"
  File "..\Win\Release\FrotzDeutsch.dll"
  File "..\Win\Release\FrotzEspañol.dll"
  File "..\Win\Release\FrotzFrançais.dll"
  File "..\Win\Release\FrotzItaliano.dll"
  File "..\Win\Release\Frotz.chm"
  File "..\Win\Release\ScaleGfx.dll"
  File "..\COPYING"
!ifdef BUNDLE_GAME
  File "Windows Frotz\obtaining_source_code.txt"
!endif

  SetOutPath "$INSTDIR\Examples"

  File "..\Examples\Parrot.zlb"
  File "..\Examples\Parrot.inf"
  File "..\Examples\Unicode.inf"
  File "..\Examples\Unicode.z5"
  File "..\Examples\ZSpec11.inf"
  File "..\Examples\ZSpec11.z6"

!ifdef BUNDLE_GAME
  SetOutPath "$INSTDIR\Games"
  File "Windows Frotz\Games\${GAME_FILE}"
!endif

  ; Write the uninstall keys for Windows
!ifdef BUNDLE_GAME  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsFrotz" "DisplayName" "${GAME_NAME} and Windows Frotz (remove only)"
!else
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsFrotz" "DisplayName" "Windows Frotz"
!endif  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsFrotz" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  SetOutPath "$INSTDIR"
  WriteUninstaller "Uninstall.exe"

  ; Write out IFDB keys so that the IFDB Download Advisor can find us
  WriteRegStr HKLM "Software\IFDB.tads.org\MetaInstaller\Interpreters\WindowsFrotz.DavidKinder" "Version" ${FROTZ_VERSION}
  WriteRegStr HKLM "Software\IFDB.tads.org\MetaInstaller\Interpreters\WindowsFrotz.DavidKinder" "RunGame" '"$INSTDIR\Frotz.exe" "%1"'
  WriteRegStr HKCU "Software\IFDB.tads.org\MetaInstaller\Interpreters\WindowsFrotz.DavidKinder" "Version" ${FROTZ_VERSION}
  WriteRegStr HKCU "Software\IFDB.tads.org\MetaInstaller\Interpreters\WindowsFrotz.DavidKinder" "RunGame" '"$INSTDIR\Frotz.exe" "%1"'

SectionEnd ; end the section

Section "Start Menu Shortcuts"

  SectionIn RO

  StrCpy $R9 ""

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

  StrCpy $R0 $STARTMENU_FOLDER
  StrCpy $R9 "$SMPROGRAMS\$R0"
  IfFileExists "$R9" startmenuFolderExists
  CreateDirectory "$R9"
startmenuFolderExists:
  CreateShortCut "$R9\Windows Frotz.lnk" "$INSTDIR\Frotz.exe"  
  CreateShortCut "$R9\Windows Frotz Help.lnk" "$INSTDIR\Frotz.chm"  
  CreateShortCut "$R9\(www) Download games for Windows Frotz.lnk" "http://www.ifarchive.org/indexes/if-archiveXgamesXzcode.html"  

!ifdef BUNDLE_GAME
  CreateShortCut "$R9\Play ${GAME_NAME}.lnk" "$INSTDIR\Frotz.exe" '"$INSTDIR\Games\${GAME_FILE}"'
  CreateShortCut "$R9\(www) Interactive Fiction Beginner's Guide.lnk" "http://ifguide.ramsberg.net"
  CreateShortCut "$R9\Uninstall ${GAME_NAME} and Windows Frotz.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0
!else
  CreateShortCut "$R9\Uninstall Windows Frotz.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0
!endif

  !insertmacro MUI_STARTMENU_WRITE_END

  ; Write the start menu path into the registry
  WriteRegStr HKLM SOFTWARE\WindowsFrotz "Startmenu_Folder" $R9
  WriteRegStr HKCU SOFTWARE\WindowsFrotz "Startmenu_Folder" $R9
  WriteRegStr HKLM SOFTWARE\WindowsFrotz "Startmenu_Folder_Name" $STARTMENU_FOLDER

  ; Delete old info in registry
  StrCmp "$InstallFor" "all" +1 initSingleUser
  StrCpy $0 0
loopUsers:
    EnumRegKey $1 HKU "" $0
    StrCmp $1 "" done
    ClearErrors
    ReadRegDWORD $2 HKU "$1\Software\David Kinder\Frotz\Files" "Register File Types"
    IfErrors frotzNotPresent
      WriteRegDWORD HKU "$1\Software\David Kinder\Frotz\Files" "Register File Types" 0
frotzNotPresent:      
    IntOp $0 $0 + 1
    goto loopUsers
initSingleUser:
    ClearErrors
    ReadRegDWORD $2 HKCU "Software\David Kinder\Frotz\Files" "Register File Types"
    IfErrors done
      WriteRegDWORD HKCU "Software\David Kinder\Frotz\Files" "Register File Types" 0
done:

SectionEnd

Var Extension
Var Zversion
Var ZRegSection
Var ZDescription

Function AddAssoc

  ; back up old value of .z?
  ReadRegStr $1 HKCR "$Extension" ""
  StrCmp $1 "" noPreviousAssoc
    StrCmp $1 "$ZRegSection" noPreviousAssoc
      WriteRegStr HKCR "$Extension" "backup_val" $1
noPreviousAssoc:
  WriteRegStr HKCR "$Extension" "" "$ZRegSection"
  WriteRegStr HKCR "$ZRegSection" "" "$ZDescription"
  WriteRegStr HKCR "$ZRegSection\shell" "" "open"
  WriteRegStr HKCR "$ZRegSection\DefaultIcon" "" '"$INSTDIR\Frotz.exe",$Zversion'
  WriteRegStr HKCR "$ZRegSection\shell\open\command" "" '"$INSTDIR\Frotz.exe" "%1"'
  
FunctionEnd

Section "File Associations"

  SectionIn RO

  IntOp $Zversion 1 + 0

  ReadINIStr $R1 "$PLUGINSDIR\WindowsFrotz.ini" "Field 1" "State" ; Reads the user's .z? file association selection
  StrCmp $R1 "0" noZAssoc

nextZversion:
  StrCpy $Extension ".z$Zversion"
  StrCpy $ZRegSection "ZMachine.V$Zversion"
  StrCpy $ZDescription "Z-code V$Zversion Adventure"
  Call AddAssoc
  IntOp $Zversion $Zversion + 1
  IntCmp $Zversion 8 nextZversion nextZversion ; The number should be the highest available Z-code version
  
noZAssoc:

  IntOp $Zversion 9 + 0
  StrCpy $ZRegSection "ZMachine.Blorb"
  StrCpy $ZDescription "Blorbed Z-code Adventure"

  ReadINIStr $R2 "$PLUGINSDIR\WindowsFrotz.ini" "Field 2" "State" ; Reads the user's .zlb file association selection
  StrCmp $R2 "0" noZlbAssoc
    StrCpy $Extension ".zlb"
    Call AddAssoc

noZlbAssoc:

  ReadINIStr $R2 "$PLUGINSDIR\WindowsFrotz.ini" "Field 3" "State" ; Reads the user's .zblorb file association selection
  StrCmp $R2 "0" noZblorbAssoc
    StrCpy $Extension ".zblorb"
    Call AddAssoc

noZblorbAssoc:

  ReadINIStr $R2 "$PLUGINSDIR\WindowsFrotz.ini" "Field 4" "State" ; Reads the user's .dat file association selection
  StrCmp $R2 "0" noDatAssoc
    IntOp $Zversion 0 + 0
    StrCpy $Extension ".dat"
    StrCpy $ZRegSection "ZMachine.Infocom"
    StrCpy $ZDescription "Infocom Z-code Adventure"
    Call AddAssoc

noDatAssoc:

SectionEnd


;--------------------------------------------------------------------------------
; Uninstaller stuff

Function un.getUserType
	ClearErrors
	UserInfo::GetName
	IfErrors Win9x
	Pop $0
	UserInfo::GetAccountType
	Pop $UserType
        Return
	
	Win9x:
		# This one means you don't need to care about admin or
		# not admin because Windows 9x doesn't either
	   StrCpy $UserType "Admin"
FunctionEnd

Function un.RemoveAssoc
  ReadRegStr $1 HKCR "$Extension" ""
  StrCpy $2 $1 9 ; Copy first 9 characters from $1 to $2
  StrCmp $2 "ZMachine." 0 NoOwn ; only do this if we own it
    ReadRegStr $3 HKCR "$Extension" "backup_val"
    StrCmp $3 "" 0 RestoreBackup ; if backup == "" then delete the whole key
      DeleteRegKey HKCR "$Extension" ; Delete .z? section
    Goto DeleteZSection
  RestoreBackup:
      WriteRegStr HKCR "$Extension" "" $3
      DeleteRegValue HKCR "$Extension" "backup_val"
  DeleteZSection:
      DeleteRegKey HKCR "$1" ; Delete ZMachine.? section 
  NoOwn:
FunctionEnd

Section "Uninstall"

ReadRegStr $R9 HKLM SOFTWARE\WindowsFrotz "Startmenu_Folder"
  StrCmp $R1 "" 0 +1
    ReadRegStr $R9 HKCU SOFTWARE\WindowsFrotz "Startmenu_Folder"

ReadRegStr $R1 HKLM SOFTWARE\WindowsFrotz "InstallFor"
  StrCmp $R1 "all" +1 noProblemUninstall
    Call un.getUserType
    StrCmp $UserType "Admin" allUninstallOk
    MessageBox MB_OK "You need administrator rights to uninstall Windows Frotz."
    Abort
allUninstallOk:  
  SetShellVarContext all
  
noProblemUninstall:  
  ; remove files and uninstaller
  Delete "$INSTDIR\Frotz.exe"
  Delete "$INSTDIR\FrotzDeutsch.dll"
  Delete "$INSTDIR\FrotzEspañol.dll"
  Delete "$INSTDIR\FrotzFrançais.dll"
  Delete "$INSTDIR\FrotzItaliano.dll"
  Delete "$INSTDIR\Frotz.chm"
  Delete "$INSTDIR\ScaleGfx.dll"
  Delete "$INSTDIR\COPYING"
  Delete "$INSTDIR\Uninstall.exe"

  Delete "$INSTDIR\Examples\Parrot.zlb"
  Delete "$INSTDIR\Examples\Parrot.inf"
  Delete "$INSTDIR\Examples\Unicode.inf"
  Delete "$INSTDIR\Examples\Unicode.z5"
  Delete "$INSTDIR\Examples\ZSpec11.inf"
  Delete "$INSTDIR\Examples\ZSpec11.z6"

!ifdef BUNDLE_GAME
  Delete "$INSTDIR\obtaining_source_code.txt"
  Delete "$INSTDIR\Games\${GAME_FILE}"
  RMDir "$INSTDIR\Games"
!endif

  ; remove shortcuts, if any
  StrCmp $R9 "" skipRemoveStart
  Delete "$R9\Windows Frotz.lnk"
  Delete "$R9\Windows Frotz Help.lnk"
  Delete "$R9\(www) Download games for Windows Frotz.lnk"
  Delete "$R9\Uninstall Windows Frotz.lnk"

!ifdef BUNDLE_GAME
  Delete "$R9\Play ${GAME_NAME}.lnk"
  Delete "$R9\Uninstall ${GAME_NAME} and Windows Frotz.lnk"
  Delete "$R9\(www) Interactive Fiction Beginner's Guide.lnk"
!else
  Delete "$R9\Uninstall Windows Frotz.lnk"
!endif

  ; remove directories used
  RMDir "$R9"
skipRemoveStart:
  RMDir "$INSTDIR\Examples"
  RMDir "$INSTDIR"

  ; remove registry keys

  IntOp $Zversion 1 + 0

nextZversionUn:
  StrCpy $Extension ".z$Zversion"
  Call un.RemoveAssoc
  IntOp $Zversion $Zversion + 1
  IntCmp $Zversion 8 nextZversionUn nextZversionUn ; The number should be the highest available Z-code version
  
  StrCpy $Extension ".zlb"
  Call un.RemoveAssoc

  StrCpy $Extension ".zblorb"
  Call un.RemoveAssoc

  StrCpy $Extension ".dat"
  Call un.RemoveAssoc

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WindowsFrotz"
  DeleteRegKey HKLM "SOFTWARE\WindowsFrotz"

  DeleteRegKey HKLM "Software\IFDB.tads.org\MetaInstaller\Interpreters\WindowsFrotz.DavidKinder"
  DeleteRegKey HKCU "Software\IFDB.tads.org\MetaInstaller\Interpreters\WindowsFrotz.DavidKinder"

SectionEnd

