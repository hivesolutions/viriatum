:: turns off the echo
@echo off

:: sets the various path related global
:: variable with relative names
set CURRENT_DIR=%cd%
set BUILD_DIR=%CURRENT_DIR%/build
set REPO_DIR=%BUILD_DIR%/repo
set BASE_DIR=%REPO_DIR%
set SRC_DIR=%BASE_DIR%/bin/hive_viriatum/i386/win32/Release
set SOLUTION_DIR=%BASE_DIR%/win32/vs2008ex

:: sets the version string constant value
set VERSION=0.1.0

git clone git://github.com/hivesolutions/viriatum.git %REPO_DIR%

msbuild %SOLUTION_DIR%/hive_viriatum.sln /p:Configuration=Release
cd %SRC_DIR%
tar -cf viriatum_%VERSION%.tar index.html viriatum.exe resources templates
cd %BUILD_DIR%
capsule clone viriatum-%VERSION%.exe
capsule extend viriatum-%VERSION%.exe Viriatum "Viriatum HTTP Server" %SRC_DIR%/viriatum_%VERSION%.tar
