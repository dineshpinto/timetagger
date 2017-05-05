!define PRODUCT_NAME "TimeTagger"
!define PRODUCT_VERSION "0.9"
!define PRODUCT_KEY "Software\SwabianInstruments\${PRODUCT_NAME}"
!define PRODUCT_ROOT_KEY "HKLM"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

SetCompressor lzma

; MUI 1.67 compatible ------
!include "MUI.nsh"
!include "windows_version.nsh"
!include "driver.nsh"
!include "WinVer.nsh"
!include x64.nsh

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; Components page
!insertmacro MUI_PAGE_COMPONENTS
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "TimeTagger-0.9.exe"
InstallDir "$PROGRAMFILES\SwabianInstruments\TimeTagger"
ShowInstDetails show
ShowUnInstDetails show

Section "opalkelly driver" SEC01

  MessageBox MB_OK "Please make sure to unplug the TimeTagger device prior continue."

  SetOverwrite ifnewer

  SetOutPath "$INSTDIR\firmware"
  File "..\backend\TimeTaggerController.bit"

  SetOutPath "$INSTDIR\win32\driver"
  File "..\win32\prequesits\Driver\i386\WdfCoInstaller01009.dll"
  File "..\win32\prequesits\Driver\i386\WinUSBCoInstaller2.dll"
  File "..\win32\prequesits\Driver\i386\okusb.cat"
  File "..\win32\prequesits\Driver\i386\okusb.inf"
  File "..\win32\prequesits\okFrontPanel.dll"
  
  SetRegView 32
  WriteRegStr ${PRODUCT_ROOT_KEY} ${PRODUCT_KEY} "InstDir" "$INSTDIR"

  ${If} ${RunningX64}
    SetOutPath "$INSTDIR\win64\driver"
    File "..\win64\prequesits\Driver\amd64\WdfCoInstaller01009.dll"
    File "..\win64\prequesits\Driver\amd64\WinUSBCoInstaller2.dll"
    File "..\win64\prequesits\Driver\amd64\okusb.cat"
    File "..\win64\prequesits\Driver\amd64\okusb.inf"
    File "..\win64\prequesits\okFrontPanel.dll"
  
    SetRegView 64
    WriteRegStr ${PRODUCT_ROOT_KEY} ${PRODUCT_KEY} "InstDir" "$INSTDIR"
  ${EndIf}

  call install_ok_driver

  SetOutPath "$INSTDIR\win32"
  File "..\win32\prequesits\vcredist_x86.exe"
  ${If} ${IsWinXP}
    ExecWait '"$INSTDIR\win32\vcredist_x86.exe" /norestart /q'
  ${Else}
    ExecWait '"$INSTDIR\win32\vcredist_x86.exe" /norestart /q' $0
  ${EndIf}
  IfErrors 0 +2
  	Abort "Install of the 3rd party component vcredist_x86.exe failed."
; the current vcredist_x86 requires windows installer >= 3.1, which is apparently not available on a fresh XP-SP2. We should at least check whether the
; vcredist process fails and inform the user about the requirement if necessary

  SetOutPath "$INSTDIR\win64"
  File "..\win64\prequesits\vcredist_x64.exe"
  ExecWait '"$INSTDIR\win64\vcredist_x64.exe" /norestart /q'
;  IfErrors 0 +2
;  	Abort "Install of the 3rd party component vcredist_x64.exe failed."



SectionEnd


;Section "C++ API" SEC02
;  SetOverwrite on
;
;  SetOutPath "$DESKTOP\TimeTagger\C++\countrate"
;  File "..\c++\Countrate\countrate.cpp"
;  File "..\c++\Countrate\Countrate.vcxproj"
;  File "..\c++\Countrate\Countrate.vcxproj.filters"
;
;  SetOutPath "$DESKTOP\TimeTagger\C++\include"
;  File "..\c++\include\Correlation.cpp"
;  File "..\c++\include\CountBetweenMarkers.cpp"
;  File "..\c++\include\Counter.cpp"
;  File "..\c++\include\SSRTimeTrace.cpp"
;  File "..\c++\include\Histogram.cpp"
;  File "..\c++\include\HWClock.h"
;  File "..\c++\include\Iterator.cpp"
;  File "..\c++\include\Logger.h"
;  File "..\c++\include\Thread.h"
;  File "..\c++\include\TimeTagger.h"
;
;  SetOutPath "$DESKTOP\TimeTagger\C++\lib"
;  File "..\c++\lib\TimeTagger-dbg.lib"
;  File "..\c++\lib\TimeTagger.lib"
;
;  SetOutPath "$DESKTOP\TimeTagger\C++"
;  File "..\c++\vstudio.sln"
;
;  SetOutPath "$INSTDIR\C++\countrate"
;  File "..\c++\Countrate\countrate.cpp"
;  File "..\c++\Countrate\Countrate.vcxproj"
;  File "..\c++\Countrate\Countrate.vcxproj.filters"
;
;  SetOutPath "$INSTDIR\C++\include"
;  File "..\c++\include\Correlation.cpp"
;  File "..\c++\include\CountBetweenMarkers.cpp"
;  File "..\c++\include\Counter.cpp"
;  File "..\c++\include\SSRTimeTrace.cpp"
;  File "..\c++\include\Histogram.cpp"
;  File "..\c++\include\HWClock.h"
;  File "..\c++\include\Iterator.cpp"
;  File "..\c++\include\Logger.h"
;  File "..\c++\include\Thread.h"
;  File "..\c++\include\TimeTagger.h"
;
;  SetOutPath "$INSTDIR\C++\lib"
;  File "..\c++\lib\TimeTagger-dbg.lib"
;  File "..\c++\lib\TimeTagger.lib"
;
;  SetOutPath "$INSTDIR\C++"
;  File "..\c++\vstudio.sln"
;
;  File "..\c++\TimeTagger-1.0-api.chm"
;  File "..\c++\TimeTagger-1.0-api.pdf"
;
;  SetOutPath "$INSTDIR"
;  File "..\TimeTaggerManual.pdf"
;
;  CreateDirectory "$SMPROGRAMS\TimeTagger"
;  CreateShortCut "$SMPROGRAMS\TimeTagger\c++.lnk" "$INSTDIR\C++"
;  CreateShortCut "$SMPROGRAMS\TimeTagger\User Manual.lnk" "$INSTDIR\TimeTaggerManual.pdf"
;
;  CreateDirectory "$SMPROGRAMS\TimeTagger\Documentation\PDF"
;  CreateShortCut "$SMPROGRAMS\TimeTagger\Documentation\PDF\TimeTagger-1.0-api.lnk" "$INSTDIR\C++\TimeTagger-1.0-api.pdf"
;
;  CreateDirectory "$SMPROGRAMS\TimeTagger\Documentation"
;  CreateShortCut "$SMPROGRAMS\TimeTagger\Documentation\TimeTagger-1.0-api.lnk" "$INSTDIR\C++\TimeTagger-1.0-api.chm"
;
;
;SectionEnd

Section "Python bindings" SEC03

  SetOverwrite on

  SetOutPath "$INSTDIR\win32\python27"

  File "..\win32\backend\_TimeTagger.pyd"
  File "..\backend\TimeTagger.py"

  SetRegView 32
  WriteRegStr ${PRODUCT_ROOT_KEY} "Software\Python\PythonCore\2.7\PythonPath\TimeTagger_new" "" "$INSTDIR\win32\python27"
  
  ${If} ${RunningX64}
    SetOutPath "$INSTDIR\win64\python27"
    File "..\win64\backend\_TimeTagger.pyd"
    File "..\backend\TimeTagger.py"
    SetRegView 64
    WriteRegStr ${PRODUCT_ROOT_KEY} "Software\Python\PythonCore\2.7\PythonPath\TimeTagger_new" "" "$INSTDIR\win64\python27"
  ${EndIf}

;  File "..\python\TimeTagger-1.0-python.chm"
;  File "..\python\TimeTagger-1.0-python.pdf"

;  CreateDirectory "$SMPROGRAMS\TimeTagger"
;  CreateShortCut "$SMPROGRAMS\TimeTagger\python.lnk" "$INSTDIR\python"

;  CreateDirectory "$SMPROGRAMS\TimeTagger\Documentation\PDF"
;  CreateShortCut "$SMPROGRAMS\TimeTagger\Documentation\PDF\TimeTagger-1.0-python.lnk" "$INSTDIR\python\TimeTagger-1.0-python.pdf"

;  CreateDirectory "$SMPROGRAMS\TimeTagger\Documentation"
;  CreateShortCut "$SMPROGRAMS\TimeTagger\Documentation\TimeTagger-1.0-python.lnk" "$INSTDIR\C++\TimeTagger-1.0-python.chm"

SectionEnd

;Section "TmeTagger Webserver" SEC04
;
;  SetOverwrite on
;
;  SetOutPath "$INSTDIR"
;  File "..\timetaggerd.exe"
;  CreateShortCut "$SMPROGRAMS\TimeTagger\Webserver.lnk" "$INSTDIR\timetaggerd"
;
;SectionEnd

Section "Documentation and Examples" SEC05

  SetOverwrite on

  SetOutPath "$INSTDIR\examples\python\quickstart"
  File "..\examples\python\quickstart\quickstart.py"

  SetOutPath "$INSTDIR\doc"
  File "..\doc\TimeTaggerManual.pdf"

SectionEnd

Function install_ok_driver
  Push "USB\VID_151F&PID_0020"
  call install_ok_driver_p
  Push "USB\VID_151F&PID_0021"
  call install_ok_driver_p
  Push "USB\VID_151F&PID_0022"
  call install_ok_driver_p
  Push "USB\VID_151F&PID_0023"
  call install_ok_driver_p
  Push "USB\VID_151F&PID_0024"
  call install_ok_driver_p
  Push "USB\VID_151F&PID_0025"
  call install_ok_driver_p
  Push "USB\VID_151F&PID_0028"
  call install_ok_driver_p
  Push "USB\VID_151F&PID_0029"
  call install_ok_driver_p
  Push "USB\VID_151F&PID_002A"
  call install_ok_driver_p
  Push "USB\VID_151F&PID_002B"
  call install_ok_driver_p
FunctionEnd

Function install_ok_driver_p
  Pop $R0
  Push "$INSTDIR\win32\driver"
  Push "$INSTDIR\win32\driver\okusb.inf"
  Push $R0
  call InstallUpgradeDriver
  ${If} ${RunningX64}
    Push "$INSTDIR\win64\driver"
    Push "$INSTDIR\win64\driver\okusb.inf"
    Push $R0
    call InstallUpgradeDriver
  ${EndIf}
FunctionEnd

Section -AdditionalIcons
  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\TimeTagger\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} ""
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} "include files and import libraries for Visual Studio"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03} "python package for Python 2.7"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC04} "TimeTagger Webserver"
!insertmacro MUI_FUNCTION_DESCRIPTION_END


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete "$INSTDIR\uninst.exe"

  RmDir /r "$INSTDIR"
  RmDir /r "$DESKTOP\TimeTagger"
  RmDir /r "$SMPROGRAMS\TimeTagger"
  RmDir /r "$APPDATA\TimeTagger"

  SetRegView 32
  DeleteRegKey ${PRODUCT_ROOT_KEY} "Software\Python\PythonCore\2.7\PythonPath\TimeTagger"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey ${PRODUCT_ROOT_KEY} "${PRODUCT_KEY}"
  
  SetRegView 64
  DeleteRegKey ${PRODUCT_ROOT_KEY} "Software\Python\PythonCore\2.7\PythonPath\TimeTagger"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey ${PRODUCT_ROOT_KEY} "${PRODUCT_KEY}"
  SetAutoClose true
SectionEnd
