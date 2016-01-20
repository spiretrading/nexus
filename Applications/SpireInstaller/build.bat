@echo off
if "%1" == "help" GOTO help
if "%1" == "-h" GOTO help

:make
  if not exist "%ProgramFiles(x86)%\NSIS" (
    echo NSIS not found.  NSIS is required to build the installer.
    echo Use help or -h for more infomation.
    GOTO done
  )
	
  if "%1" == "" (
    echo Missing version parameter.
    echo Usage: build [version]
    GOTO done
  )

  "%ProgramFiles(x86)%\NSIS\makensis.exe" "/DWORKINGDIR"="./" "/DOUTPUT_FOLDER"="./" "/DBUILD"="%1" ./InstallerScript.nsi
  GOTO done

:help
  echo Usage: build [version]
  echo   version  - The current version number.
:done
