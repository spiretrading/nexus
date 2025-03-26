ManifestDPIAware true
!define PRODUCT_NAME "Spire"
!define PRODUCT_PUBLISHER "Eidolon Systems Ltd."
!define PRODUCT_WEB_SITE "https://www.spiretrading.com"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}_Uninstall"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; Get the product version from the command line argument BUILD
!ifndef BUILD
!error "You must define BUILD on the command line. Example: makensis /DBUILD=1.0 script.nsi"
!endif
!define PRODUCT_VERSION "${BUILD}"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
Outfile "install.exe"
InstallDir "$PROGRAMFILES64\${PRODUCT_NAME}"

RequestExecutionLevel admin

!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"

!insertmacro GetParameters
!insertmacro GetOptions

; Use the product's icon
!define MUI_ICON "spire.ico"
!define MUI_UNICON "spire.ico"

; Define the installation pages
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Define the uninstallation pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

; Components
Section "Spire" SEC01
  SectionIn RO
  SetOutPath "$INSTDIR"
  File "Spire.exe"

  ; Install Visual C++ Redistributable silently
  SetOutPath "$INSTDIR"
  File "VC_redist.x64.exe"
  ExecWait '"$INSTDIR\VC_redist.x64.exe" /quiet /norestart'

  ; Write uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Write registry keys for Add/Remove Programs
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "UninstallString" "$INSTDIR\\uninstall.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "DisplayIcon" "$INSTDIR\\uninstall.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegDWORD HKLM "${PRODUCT_DIR_REGKEY}" "NoModify" 1
  WriteRegDWORD HKLM "${PRODUCT_DIR_REGKEY}" "NoRepair" 1

  nsExec::ExecToLog 'icacls "$INSTDIR" /grant Users:(OI)(CI)F /T'
SectionEnd

Section "Start Menu Shortcuts" SEC02
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\Spire.exe" "" "$INSTDIR\Spire.exe" 0
SectionEnd

Section "Desktop Shortcut" SEC03
  CreateShortCut "$DESKTOP\Spire.lnk" "$INSTDIR\Spire.exe" "" "$INSTDIR\Spire.exe" 0
SectionEnd

; Uninstaller
Section "Uninstall" SEC04
  SetShellVarContext all

  ; Remove desktop and start menu shortcuts
  Delete "$DESKTOP\Spire.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk"
  RmDir /r "$SMPROGRAMS\${PRODUCT_NAME}"

  Delete "$INSTDIR\Spire.exe"
  Delete "$INSTDIR\VC_redist.x64.exe"

  RmDir /r "$INSTDIR"

  SetShellVarContext current
  RmDir /r "$LOCALAPPDATA\Eidolon Systems"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_DIR_REGKEY}"
SectionEnd

; Define language strings for section descriptions
LangString DESC_SEC01 ${LANG_ENGLISH} "Installs the Spire desktop trading application and prerequisites."
LangString DESC_SEC02 ${LANG_ENGLISH} "Creates Start Menu shortcuts for launching Spire and for uninstalling the application."
LangString DESC_SEC03 ${LANG_ENGLISH} "Creates a desktop shortcut for Spire."
LangString DESC_SEC04 ${LANG_ENGLISH} "Removes application files and shortcuts for Spire."

; Insert the description macros
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} $(DESC_SEC01)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} $(DESC_SEC02)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03} $(DESC_SEC03)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC04} $(DESC_SEC04)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function .onInit
  SetShellVarContext all
FunctionEnd
