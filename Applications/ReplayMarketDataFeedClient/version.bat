@ECHO OFF
SETLOCAL
IF NOT EXIST Version.hpp (
  COPY NUL Version.hpp > NUL
)
FOR /f "usebackq tokens=*" %%a IN (`git --git-dir=%~dp0..\..\.git rev-list --count --first-parent HEAD`) DO SET VERSION=%%a
findstr "%VERSION%" Version.hpp > NUL
IF NOT "%ERRORLEVEL%" == "0" (
  ECHO #define REPLAY_MARKET_DATA_FEED_CLIENT_VERSION "%VERSION%"> Version.hpp
)
EXIT /B 0
ENDLOCAL
