ManifestDPIAware true
!define PRODUCT_NAME "Spire"
!define PRODUCT_PUBLISHER "Eidolon Systems Ltd."
!define PRODUCT_WEB_SITE "https://www.spiretrading.com"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKCU"

; Get the product version from the command line argument BUILD
!ifndef BUILD
!error "You must define BUILD on the command line. Example: makensis /DBUILD=1.0 script.nsi"
!endif
!define PRODUCT_VERSION "${BUILD}"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
Outfile "install.exe"
InstallDir "$LOCALAPPDATA\${PRODUCT_NAME}"
InstallDirRegKey HKCU "${PRODUCT_DIR_REGKEY}" "InstallLocation"

RequestExecutionLevel user

!include "MUI2.nsh"
!include "LogicLib.nsh"
; Use the product's icon
!define MUI_ICON "${__FILEDIR__}\spire.ico"
!define MUI_UNICON "${__FILEDIR__}\spire.ico"

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
  ; Install Visual C++ Redistributable silently
  InitPluginsDir
  SetOutPath "$PLUGINSDIR"
  File "VC_redist.x64.exe"
  ClearErrors
  ExecWait '"$PLUGINSDIR\VC_redist.x64.exe" /quiet /norestart' $0
  ${If} ${Errors}
    MessageBox MB_OK|MB_ICONSTOP "Unable to start the VC++ runtime installer."
    SetErrorLevel 1
    Abort
  ${EndIf}
  ${If} $0 = 3010
  ${OrIf} $0 = 1641
    SetRebootFlag true
  ${ElseIf} $0 = 1638
  ${OrIf} $0 = 0x80070666
    SetRegView 64
    ReadRegDWORD $1 HKLM \
      "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" "Installed"
    SetRegView lastused
    ${If} $1 <> 1
      MessageBox MB_OK|MB_ICONSTOP "The required VC++ runtime is unavailable."
      SetErrorLevel 1
      Abort
    ${EndIf}
  ${ElseIf} $0 <> 0
    MessageBox MB_OK|MB_ICONSTOP "VC++ runtime installation failed ($0)."
    SetErrorLevel 1
    Abort
  ${EndIf}
  Delete "$PLUGINSDIR\VC_redist.x64.exe"
  SetOutPath "$INSTDIR"
  File "Spire.exe"

  ; Write uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Write registry keys for Add/Remove Programs
  WriteRegStr HKCU "${PRODUCT_DIR_REGKEY}" "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
  WriteRegStr HKCU "${PRODUCT_DIR_REGKEY}" "UninstallString" \
    '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKCU "${PRODUCT_DIR_REGKEY}" "DisplayIcon" "$INSTDIR\\Spire.exe"
  WriteRegStr HKCU "${PRODUCT_DIR_REGKEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKCU "${PRODUCT_DIR_REGKEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr HKCU "${PRODUCT_DIR_REGKEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKCU "${PRODUCT_DIR_REGKEY}" "InstallLocation" "$INSTDIR"
  WriteRegDWORD HKCU "${PRODUCT_DIR_REGKEY}" "EstimatedSize" 61440
  WriteRegDWORD HKCU "${PRODUCT_DIR_REGKEY}" "NoModify" 1
  WriteRegDWORD HKCU "${PRODUCT_DIR_REGKEY}" "NoRepair" 1
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
  ; Remove desktop and start menu shortcuts
  Delete "$DESKTOP\Spire.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk"
  RmDir "$SMPROGRAMS\${PRODUCT_NAME}"

  Delete "$INSTDIR\Spire.exe"
  Delete "$INSTDIR\uninstall.exe"
  SetOutPath "$TEMP"
  RmDir "$INSTDIR"

  RmDir /r "$LOCALAPPDATA\Eidolon Systems\${PRODUCT_NAME}"
  RmDir "$LOCALAPPDATA\Eidolon Systems"
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
  SetShellVarContext current
FunctionEnd
