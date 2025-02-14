@ECHO OFF
SETLOCAL EnableDelayedExpansion
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
IF "!IS_DEPENDENCY!" == "1" (
  SET DEPENDENCIES=!ARG!
  SET IS_DEPENDENCY=
  SHIFT
  GOTO begin_args
) ELSE IF NOT "!ARG!" == "" (
  IF "!ARG:~0,3!" == "-DD" (
    SET IS_DEPENDENCY=1
  )
  SHIFT
  GOTO begin_args
)
IF "!DEPENDENCIES!" == "" (
  SET DEPENDENCIES=!ROOT!\Dependencies
)
IF NOT EXIST "!DEPENDENCIES!" (
  MD "!DEPENDENCIES!"
)
PUSHD "!DEPENDENCIES!"
CALL "!DIRECTORY!..\..\Nexus\setup.bat"
POPD
IF NOT "!DEPENDENCIES!" == "!ROOT!\Dependencies" (
  IF EXIST Dependencies (
    RD /S /Q Dependencies
  )
  mklink /j Dependencies "!DEPENDENCIES!" > NUL
)
SET RUN_CMAKE=
IF NOT EXIST CMakeFiles (
  SET RUN_CMAKE=1
) ELSE (
  IF NOT EXIST CMakeFiles\timestamp.txt (
    SET RUN_CMAKE=1
  ) ELSE (
    SET TIMESTAMP_COMMAND=powershell -Command "& { " ^
      "$timestampFileTime = " ^
      "  (Get-Item 'CMakeFiles\\timestamp.txt').LastWriteTime;" ^
      "$filesToCheck = @((Get-Item '.\\CMakeLists.txt'));" ^
      "$filesToCheck += Get-ChildItem -Path '.\\Config\\*.cmake';" ^
      "$filesToCheck += Get-ChildItem -Path '.\\Config\\' -Filter " ^
      "  'CMakeLists.txt' -Recurse;" ^
      "foreach($file in $filesToCheck) {" ^
      "  $fileTime = $file.LastWriteTime;" ^
      "  if($fileTime -ge $timestampFileTime) {" ^
      "    Write-Output '1';" ^
      "    Exit;" ^
      "  }" ^
      "};" ^
      "Exit;" ^
    "}"
    FOR /F "delims=" %%A IN ('CALL !TIMESTAMP_COMMAND!') DO SET RUN_CMAKE=%%A
  )
)
IF "!RUN_CMAKE!" == "1" (
  IF NOT EXIST CMakeFiles (
    MD CMakeFiles
  )
  ECHO timestamp > CMakeFiles\timestamp.txt
)
IF EXIST "!DIRECTORY!Include" (
  DIR /a-d /b /s "!DIRECTORY!Include\*" > hpp_hash.txt
  FOR /F %%i IN ('certutil -hashfile hpp_hash.txt SHA256 ^| ^
      find /v "CertUtil" ^| find /v "SHA256 hash"') DO (
    IF EXIST CMakeFiles\hpp_hash.txt (
      FOR /F %%j IN (CMakeFiles\hpp_hash.txt) DO (
        IF NOT "%%i" == "%%j" (
          SET RUN_CMAKE=1
        )
      )
    ) ELSE (
      SET RUN_CMAKE=1
    )
    IF "!RUN_CMAKE!" == "1" (
      ECHO %%i > CMakeFiles\hpp_hash.txt
    )
  )
  DEL hpp_hash.txt
)
IF EXIST "!DIRECTORY!Source" (
  DIR /a-d /b /s "!DIRECTORY!Source\*" > cpp_hash.txt
  FOR /F %%i IN ('certutil -hashfile cpp_hash.txt SHA256 ^| ^
      find /v "CertUtil" ^| find /v "SHA256 hash"') DO (
    IF EXIST CMakeFiles\cpp_hash.txt (
      FOR /F %%j IN (CMakeFiles\cpp_hash.txt) DO (
        IF NOT "%%i" == "%%j" (
          SET RUN_CMAKE=1
        )
      )
    ) ELSE (
      SET RUN_CMAKE=1
    )
    IF "!RUN_CMAKE!" == "1" (
      ECHO %%i > CMakeFiles\cpp_hash.txt
    )
  )
  DEL cpp_hash.txt
)
CALL !DIRECTORY!version.bat
IF "!RUN_CMAKE!" == "1" (
  cmake -S !DIRECTORY! -DD=!DEPENDENCIES!
)
EXIT /B 0
ENDLOCAL
