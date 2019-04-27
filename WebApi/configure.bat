@ECHO OFF
SETLOCAL
SET ROOT=%cd%
SET DEPENDENCIES=
SET IS_DEPENDENCY=
:begin_args
SET ARG=%~1
IF "%IS_DEPENDENCY%" == "1" (
  SET DEPENDENCIES=%ARG%
  SET IS_DEPENDENCY=
  GOTO begin_args
) ELSE IF NOT "%ARG%" == "" (
  IF "%ARG:~0,3%" == "-DD" (
    SET IS_DEPENDENCY=1
  )
  SHIFT
  GOTO begin_args
)
IF "%DEPENDENCIES%" == "" (
  SET DEPENDENCIES=%ROOT%\..\Dependencies
)
IF NOT EXIST build.bat (
  ECHO CALL "%~dp0build.bat" %%* > build.bat
)
IF NOT EXIST "%DEPENDENCIES%" (
  MD "%DEPENDENCIES%"
)
IF EXIST Dependencies (
  RMDIR Dependencies /S /Q
)
mklink /j Dependencies "%DEPENDENCIES%"
PUSHD "%DEPENDENCIES%"
CALL "%~dp0setup.bat"
POPD
ENDLOCAL
