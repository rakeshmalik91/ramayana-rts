!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "x64.nsh"
!include "MUI.nsh"

#-------------------------------------------------------------------------------------------application descriptiuon

!define APPNAME "Ramayana"
!define COMPANYNAME "Ramayana"
!define DESCRIPTION "An RTS game based on Indian great epic Ramayana"
!define VERSIONMAJOR 1
!define VERSIONMINOR 0
!define VERSIONBUILD 0
!define HELPURL "http://steamcommunity.com/sharedfiles/filedetails/?id=231706961"
!define UPDATEURL "http://steamcommunity.com/sharedfiles/filedetails/?id=231706961"
!define ABOUTURL "http://steamcommunity.com/sharedfiles/filedetails/?id=231706961"

#-------------------------------------------------------------------------------------------attributes

Name ${APPNAME}
OutFile "distributable\Install ${APPNAME} Test.exe"
InstallDir $PROGRAMFILES\${APPNAME}

#-------------------------------------------------------------------------------------------MUI interface

!define MUI_ICON "Install.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "Install.ico"
!define MUI_HEADERIMAGE_RIGHT

!define MUI_UNICON "Uninstall.ico"
!define MUI_UNHEADERIMAGE
!define MUI_UNHEADERIMAGE_BITMAP "Uninstall.ico"
!define MUI_UNHEADERIMAGE_RIGHT

#-------------------------------------------------------------------------------------------Install pages

!define MUI_WELCOMEPAGE_TITLE "Welcome"
!define MUI_WELCOMEPAGE_TEXT "This will install ${APPNAME} Demo in computer.\n\nIt is a real time strategy game based on Indian great epic Ramayana.\n\nThis version is a demo version of the original game and contains only a small part of the game. The graphics and AI may not match the original full version of the game.\n\nClick Next to continue or Cancel to exit setup."
!insertmacro MUI_PAGE_WELCOME

!insertmacro MUI_PAGE_LICENSE "license.rtf"

!insertmacro MUI_PAGE_COMPONENTS

!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_TITLE "Installation Completed"
!define MUI_FINISHPAGE_TEXT "You have successfully installed ${APPNAME}."
!insertmacro MUI_PAGE_FINISH

#-------------------------------------------------------------------------------------------Uninstall pages

!define MUI_WELCOMEPAGE_TITLE "Uninstall ${APPNAME}"
!define MUI_WELCOMEPAGE_TEXT "You are about to uninstall ${APPNAME}."
!insertmacro MUI_UNPAGE_WELCOME

!insertmacro MUI_UNPAGE_CONFIRM 

!insertmacro MUI_UNPAGE_COMPONENTS

!insertmacro MUI_UNPAGE_INSTFILES 

!define MUI_FINISHPAGE_TITLE "Uninstallation Completed"
!define MUI_FINISHPAGE_TEXT "You have successfully uninstalled ${APPNAME}."
!insertmacro MUI_UNPAGE_FINISH

#-------------------------------------------------------------------------------------------MUI language

!insertmacro MUI_LANGUAGE "English"

#-------------------------------------------------------------------------------------------Install section

Section "Application Data"
	SectionIn RO

	SetOutPath $INSTDIR
	${If} ${RunningX64}
		File /oname=${APPNAME}.exe "${APPNAME}.x64.exe"
	${Else}
		File /oname=${APPNAME}.exe "${APPNAME}.Win32.exe"
	${EndIf}
	
	File "${APPNAME}.ico" "${APPNAME}_small.ico" "Uninstall.ico"
SectionEnd
	
Section "Uninstaller"
	SectionIn RO

	SetOutPath $INSTDIR
	WriteUninstaller "$INSTDIR\Uninstall.exe"
	
	# Registry information for add/remove programs
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME} - ${DESCRIPTION}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "InstallLocation" "$\"$INSTDIR$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayIcon" "$\"$INSTDIR\Uninstall.ico$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "Publisher" "$\"${COMPANYNAME}$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "HelpLink" "$\"${HELPURL}$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "URLUpdateInfo" "$\"${UPDATEURL}$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "URLInfoAbout" "$\"${ABOUTURL}$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayVersion" "$\"${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}$\""
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMajor" ${VERSIONMAJOR}
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMinor" ${VERSIONMINOR}
	# There is no option for modifying or repairing the install
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoRepair" 1
SectionEnd
	
Section "Desktop Shortcut"
	SetOutPath $INSTDIR
	CreateShortCut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\Ramayana.exe"
SectionEnd

Section "Start Menu Shortcut"
	CreateDirectory "$SMPROGRAMS\${APPNAME}"
	CreateShortCut "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}.lnk" "$INSTDIR\${APPNAME}.exe"
	CreateShortCut "$SMPROGRAMS\${COMPANYNAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

#-------------------------------------------------------------------------------------------Uninstall section

Section "UN.Save Files" 
	RMDir /r "$INSTDIR\save"
SectionEnd

Section "UN.Log Files" 
	RMDir /r "$INSTDIR\log"
SectionEnd

Section "UN.Desktop Shortcut" 
	Delete "$DESKTOP\${APPNAME}.lnk"
SectionEnd

Section "UN.Start Menu Shortcut" 
	Delete "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}.lnk"
	Delete "$SMPROGRAMS\${COMPANYNAME}\Uninstall.lnk"
	
	RMDir "$SMPROGRAMS\${COMPANYNAME}"
SectionEnd

Section "UN.Application Data" 
	SectionIn RO

	Delete "$INSTDIR\*.dll"
	Delete "$INSTDIR\${APPNAME}.exe"
	Delete "$INSTDIR\Uninstall.exe"
	Delete "$INSTDIR\${APPNAME}.ico"
	Delete "$INSTDIR\${APPNAME}_small.ico"
	Delete "$INSTDIR\Uninstall.ico"
	RMDir /r "$INSTDIR\audio"
	RMDir /r "$INSTDIR\campaign"
	RMDir /r "$INSTDIR\cursor"
	RMDir /r "$INSTDIR\data"
	RMDir /r "$INSTDIR\kb"
	RMDir /r "$INSTDIR\shaders"
	RMDir /r "$INSTDIR\special"
	RMDir /r "$INSTDIR\map"
	RMDir /r "$INSTDIR\ui"
	RMDir /r "$INSTDIR\unit"
	RMDir /r "$INSTDIR\video"
	
	RMDir $INSTDIR
	
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
SectionEnd