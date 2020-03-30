@ECHO OFF
SETLOCAL
SET ROOT=%cd%
IF NOT EXIST build.bat (
  ECHO @ECHO OFF > build.bat
  ECHO CALL "%~dp0build.bat" %%* >> build.bat
)
IF NOT EXIST configure.bat (
  ECHO @ECHO OFF > configure.bat
  ECHO CALL "%~dp0configure.bat" %%* >> configure.bat
)
SET DIRECTORY=%~dp0
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
  SET DEPENDENCIES=%ROOT%\Dependencies
)
IF NOT EXIST "%DEPENDENCIES%" (
  MD "%DEPENDENCIES%"
)
PUSHD "%DEPENDENCIES%"
CALL "%DIRECTORY%setup.bat"
POPD
IF NOT "%DEPENDENCIES%" == "%ROOT%\Dependencies" (
  IF NOT EXIST Dependencies (
    mklink /j Dependencies "%DEPENDENCIES%" > NUL
  )
)
cmake -A Win32 -T host=x64 "%DIRECTORY%"
ENDLOCAL
