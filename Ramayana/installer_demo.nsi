!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "x64.nsh"
!include "MUI.nsh"

#-------------------------------------------------------------------------------------------application descriptiuon

!define APPNAME "Ramayana"
!define COMPANYNAME "Ramayana"
!define DESCRIPTION "An RTS game based on Indian great epic Ramayana"
!define VERSIONMAJOR 0
!define VERSIONMINOR 0
!define VERSIONBUILD 0
!define HELPURL "http://steamcommunity.com/sharedfiles/filedetails/?id=231706961"
!define UPDATEURL "http://steamcommunity.com/sharedfiles/filedetails/?id=231706961"
!define ABOUTURL "http://steamcommunity.com/sharedfiles/filedetails/?id=231706961"

#-------------------------------------------------------------------------------------------attributes

Name ${APPNAME}
OutFile "distributable\Install ${APPNAME} Demo.exe"
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

#!insertmacro MUI_PAGE_LICENSE "license.rtf"

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

Section "VCRedist 2013"
	SetOutPath $INSTDIR
	${If} ${RunningX64}
		File /oname=vcredist2013.exe lib\vcredist2013_x64.exe
	${Else}
		File /oname=vcredist2013.exe lib\vcredist2013_x86.exe
	${EndIf} 
	ExecWait "$INSTDIR\vcredist2013.exe"
SectionEnd

Section "Application Data"
	SectionIn RO
	
	SetOutPath "$INSTDIR\"
	${If} ${RunningX64}
		File lib\glew-1.9.0\bin\x64\*.dll
		File lib\freeglut\bin\x64\*.dll
		File lib\opencv-2.4.9\x64\vc12\bin\*.dll
		File lib\SDL-1.2.15\lib\x64\*.dll
		File lib\SDL_mixer-1.2.12\lib\x64\*.dll
		File lib\steamwork-sdk\public\steam\lib\win64\*.dll
		File lib\openvr-master\bin\win64\*.dll
		File /oname=${APPNAME}.exe "${APPNAME}.x64.Demo.exe"
	${Else}
		File lib\glew-1.9.0\bin\x86\*.dll
		File lib\freeglut\bin\x86\*.dll
		File lib\opencv-2.4.9\x86\vc12\bin\*.dll
		File lib\SDL-1.2.15\lib\x86\*.dll
		File lib\SDL_mixer-1.2.12\lib\x86\*.dll
		File lib\steamwork-sdk\public\steam\lib\win32\*.dll
		File lib\openvr-master\bin\win32\*.dll
		File /oname=${APPNAME}.exe "${APPNAME}.Win32.Demo.exe"
	${EndIf}  
	File "${APPNAME}.ico" "${APPNAME}_small.ico" "Uninstall.ico" 
	File /r "audio" "cursor" "kb" "shaders" "special"
	CreateDirectory "save" 
	CreateDirectory "log"
	
	# only campaign 1
	SetOutPath "$INSTDIR\campaign"
	File "campaign\01*"
	
	SetOutPath "$INSTDIR\data"
	File /oname=unit.xml "data\unit_deploy_demo.xml"
	File /oname=settings.xml "data\settings_deploy.xml"
	IfFileExists "$INSTDIR\data\campaign.xml" label_to_goto_if_file_exists
	File /oname=campaign.xml "data\campaign_deploy.xml"
	label_to_goto_if_file_exists:
	
	SetOutPath "$INSTDIR\ui"
	File "ui\*"
	SetOutPath "$INSTDIR\ui\campaign_icon"
	File "ui\campaign_icon\1.png" "ui\campaign_icon\2.png"
	SetOutPath "$INSTDIR\ui\campaign_image"
	File "ui\campaign_image\1.png"
	SetOutPath "$INSTDIR\ui\campaign_name"
	File "ui\campaign_name\1.png"
	
	# only campaign 1 units
	SetOutPath "$INSTDIR\unit" 
	File /r "unit\asvattha" "unit\bald_tree" "unit\bamboo" "unit\boar" "unit\crow" "unit\cycus" "unit\cycus2" "unit\date" "unit\deer" "unit\eagle" "unit\fern" "unit\fern2" "unit\flag_long" "unit\flamboyant" "unit\grass" "unit\grey_stone0" "unit\hut" "unit\ivy" "unit\jackfruit" "unit\jatayu" "unit\kabandha" "unit\lakshmana" "unit\lanka_swordsman" "unit\mango" "unit\mine" "unit\neem" "unit\sita_evidance" "unit\tamarind" "unit\traveler""unit\tulip" "unit\villager_female" "unit\villager_female_2" "unit\villager_male" "unit\villager_male_2"
	SetOutPath "$INSTDIR\unit\rama"
	File "unit\rama\*.png" "unit\rama\*.jpg" "unit\rama\base.unit" "unit\rama\sword.unit"
	
	SetOutPath "$INSTDIR\video"
	File /r "video\loading_game" "video\menu"
 	# only gandhamadan, rama_sword video
	SetOutPath "$INSTDIR\video\loading_map\480p"
	File "video\loading_map\480p\gandhamadan.avi" "video\loading_map\480p\rama_sword.avi"
	SetOutPath "$INSTDIR\video\loading_map\720p"
	File "video\loading_map\720p\gandhamadan.avi" "video\loading_map\720p\rama_sword.avi"
	SetOutPath "$INSTDIR\video\loading_map\1080p"
	File "video\loading_map\1080p\gandhamadan.avi" "video\loading_map\1080p\rama_sword.avi"
	# only campaign 1 cut-scenes
	SetOutPath "$INSTDIR\video\cutscene\480p"
	File "video\cutscene\480p\cmpn01_*"
	SetOutPath "$INSTDIR\video\cutscene\720p"
	File "video\cutscene\720p\cmpn01_*"
	SetOutPath "$INSTDIR\video\cutscene\1080p"
	File "video\cutscene\1080p\cmpn01_*"
	SetOutPath "$INSTDIR\video\cutscene\audio_track"
	File "video\cutscene\audio_track\cmpn01_*"
SectionEnd
	
Section "Desktop Shortcut"
	SetOutPath "$INSTDIR\"
	CreateShortCut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\${APPNAME}.exe"
SectionEnd

Section "Start Menu Shortcut"
	SetOutPath "$INSTDIR\"
	CreateDirectory "$SMPROGRAMS\${APPNAME}"
	CreateShortCut "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}.lnk" "$INSTDIR\${APPNAME}.exe"
	CreateShortCut "$SMPROGRAMS\${COMPANYNAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd
	
Section "Uninstaller"
	SectionIn RO
	
	SetOutPath "$INSTDIR\"
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

#-------------------------------------------------------------------------------------------Uninstall section

Section "UN.Save Files" 
	RMDir /r "$INSTDIR\save"
	Delete "$INSTDIR\data\campaign.xml"
SectionEnd

Section "UN.Log Files" 
	RMDir /r "$INSTDIR\log"
	Delete "$INSTDIR\stderr.txt"
	Delete "$INSTDIR\stdout.txt"
SectionEnd

Section "UN.Desktop Shortcut" 
	Delete "$DESKTOP\${APPNAME}.lnk"
SectionEnd

Section "UN.Start Menu Shortcut" 
	Delete "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}.lnk"
	Delete "$SMPROGRAMS\${COMPANYNAME}\Uninstall.lnk"
	RMDir "$SMPROGRAMS\${COMPANYNAME}"
SectionEnd

Section "UN.VCRedist 2013"
	ExecWait "$INSTDIR\vcredist2013.exe"
	Delete "$INSTDIR\vcredist2013.exe"
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
	RMDir /r "$INSTDIR\kb"
	RMDir /r "$INSTDIR\shaders"
	RMDir /r "$INSTDIR\special"
	RMDir /r "$INSTDIR\map"
	RMDir /r "$INSTDIR\ui"
	RMDir /r "$INSTDIR\unit"
	RMDir /r "$INSTDIR\video"
	Delete "$INSTDIR\data\unit.xml"
	Delete "$INSTDIR\data\settings.xml"
	RMDir "$INSTDIR\data"
	RMDir $INSTDIR
	
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
SectionEnd
