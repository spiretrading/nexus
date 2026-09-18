@ECHO OFF
SETLOCAL

PUSHD "%~dp0" || EXIT /B 1

REM Command file for Sphinx documentation

if "%SPHINXBUILD%" == "" (
	set SPHINXBUILD=sphinx-build
)
set SOURCEDIR=source
set BUILDDIR=build
set SPHINXPROJ=SpireWebServices

CALL "%SPHINXBUILD%" --version >NUL 2>NUL
if errorlevel 9009 (
	echo.
	echo.The 'sphinx-build' command was not found. Make sure you have Sphinx
	echo.installed, then set the SPHINXBUILD environment variable to point
	echo.to the full path of the 'sphinx-build' executable. Alternatively you
	echo.may add the Sphinx directory to PATH.
	echo.
	echo.If you don't have Sphinx installed, grab it from
	echo.http://sphinx-doc.org/
	POPD
	EXIT /B 1
)

if "%~1" == "" goto help

CALL "%SPHINXBUILD%" -M "%~1" "%SOURCEDIR%" "%BUILDDIR%" %SPHINXOPTS%
SET "BUILD_RESULT=%ERRORLEVEL%"
goto end

:help
CALL "%SPHINXBUILD%" -M help "%SOURCEDIR%" "%BUILDDIR%" %SPHINXOPTS%
SET "BUILD_RESULT=%ERRORLEVEL%"

:end
popd
EXIT /B %BUILD_RESULT%
