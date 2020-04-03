!include "MUI.nsh"
!include "nsDialogs.nsh"
!include "winmessages.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"

!define APPNAME "Spire"
!define COMPANYNAME "Spire Trading Inc."
!define VERSIONMAJOR 1
!define VERSIONMINOR 0

!define VERSION "${VERSIONMAJOR}.${VERSIONMINOR}.${BUILD}"
!define DESCRIPTION "Spire Trading Client Application"
!define ARP "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}"
!define MY_USERPROFILE "$%USERPROFILE%"

Name "${APPNAME} - Build ${BUILD}"
!ifndef OUTPUT_FOLDER
	!define OUTPUT_FOLDER ${DESKTOP}
!endif
OutFile "${OUTPUT_FOLDER}\install.exe"
InstallDir $PROGRAMFILES\Spire
RequestExecutionLevel highest
;Install
!insertmacro MUI_LANGUAGE English 
!insertmacro MUI_PAGE_COMPONENTS
Page directory
Page instfiles
;Uninstaller
UninstPage custom un.MyConfirmShow un.nsDialogsPageLeave
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!define MUI_FINISHPAGE_RUN
;!insertmacro MUI_UNPAGE_FINISH

function .onInit
	setShellVarContext all
functionEnd


VIProductVersion "${VERSION}.0"
VIAddVersionKey ProductName "${APPNAME}"
VIAddVersionKey CompanyName '${COMPANYNAME}'
VIAddVersionKey FileVersion 1.0.0.0
VIAddVersionKey InternalName "${VERSION}"
VIAddVersionKey OriginalFilename "${APPNAME}.exe"
VIAddVersionKey FileDescription "Spire Installer"

BrandingText "${COMPANYNAME} - ${APPNAME} Installer v${VERSION}"


var unCheckbox 
var Dialog
var checked
Function un.MyConfirmShow 
	Push 0
	Pop $9
    nsDialogs::Create 1018 
    Pop $Dialog 
    ${If} $Dialog == error 
        Abort 
    ${EndIf}
	
	${NSD_CreateCheckbox} 0% 25% 100% 10u "&Remove profile" 
    Pop $unCheckbox	
	${NSD_CreateLabel} 0% 0% 100% 10u "Choosing to remove your profile will delete your existing account profile during the unintsall."
	Pop $0  
    nsDialogs::Show 
FunctionEnd

Function  un.nsDialogsPageLeave
	${NSD_GetState} $unCheckbox $checked
FunctionEnd

Section "Spire"
	SectionIn RO
    SetOutPath $INSTDIR
	File "${WORKINGDIR}\Spire.exe"
	File "${WORKINGDIR}\VC_redist.x86.exe"
    WriteUninstaller "$INSTDIR\uninstall.exe"
	# Registry information for add/remove programs
	WriteRegStr  HKLM "${ARP}" "DisplayName" ${APPNAME}
	WriteRegStr  HKLM "${ARP}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
	WriteRegStr  HKLM "${ARP}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
	WriteRegStr  HKLM "${ARP}" "InstallLocation" "$\"$INSTDIR$\""
	WriteRegStr HKLM "${ARP}" "Publisher" '${COMPANYNAME}'
	WriteRegStr HKLM "${ARP}" "DisplayVersion" ${VERSION}
	WriteRegDWORD HKLM "${ARP}" "VersionMajor" ${VERSIONMAJOR}
	WriteRegDWORD HKLM "${ARP}" "VersionMinor" ${VERSIONMINOR}
	${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
	IntFmt $0 "0x%08X" $0
	WriteRegDWORD HKLM "${ARP}" "EstimatedSize" "$0"
	# Redistributable
	ReadRegStr $1 HKLM "SOFTWARE\Wow6432Node\Microsoft\DevDiv\vc\Servicing\14.0\RuntimeMinimum" "Install"
	StrCmp $1 1 installed
	ReadRegStr $1 HKLM "SOFTWARE\Microsoft\DevDiv\vc\Servicing\14.0\RuntimeMinimum" "Install"
	StrCmp $1 1 installed
	SetOutPath "$INSTDIR"
	File "VC_redist.x86.exe"
	ExecWait '"$INSTDIR\VC_redist.x86.exe"  /passive /norestart /s'
	Delete "$INSTDIR\VC_redist.x86.exe"
	installed:
SectionEnd

Section "Start Menu Shortcuts"
	# ShortCut links
	CreateDirectory "$SMPROGRAMS\Spire"
    CreateShortCut "$SMPROGRAMS\Spire\Spire.lnk" "$INSTDIR\Spire.exe"
	CreateShortCut "$SMPROGRAMS\Spire\uninstall.lnk" "$INSTDIR\uninstall.exe"
SectionEnd

Section "Desktop Shortcut"
	CreateShortCut "$DESKTOP\Spire.lnk" "$INSTDIR\Spire.exe"
SectionEnd

Function un.onInit
	SetShellVarContext all
	#Verify the uninstaller - last chance to back out
	MessageBox MB_OKCANCEL "Do you want to remove ${APPNAME}?" IDOK next
		Abort
	next:
FunctionEnd

Section "uninstall"
	${If} $checked == ${BST_CHECKED}
		RMDir /r "${MY_USERPROFILE}\AppData\Local\Eidolon Systems\Spire"
	${EndIf}
	Delete "$DESKTOP\Spire.lnk"
	RMDir /r $INSTDIR
	RMDir /r "$SMPROGRAMS\Spire"
	DeleteRegKey HKLM "${ARP}"
SectionEnd