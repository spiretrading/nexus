@ECHO OFF
SETLOCAL EnableDelayedExpansion
IF NOT EXIST Version.hpp (
  COPY NUL Version.hpp >NUL
)
FOR /F "usebackq tokens=*" %%a IN (`git --git-dir=%~dp0..\..\.git rev-list --count --first-parent HEAD`) DO SET VERSION=%%a
findstr "!VERSION!" Version.hpp >NUL
IF ERRORLEVEL 1 (
  >Version.hpp ECHO #define SIMULATION_ORDER_EXECUTION_SERVER_VERSION "!VERSION!"
)
EXIT /B 0
