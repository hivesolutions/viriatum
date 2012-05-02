:: turns off the echo
@echo off

:: sets the version string constant value
set VERSION=0.1.0

:: sets the various global related name values
:: (going to be used for file construction)
set ARCHITECTURE=x86
set NAME=viriatum-%VERSION%

:: sets the various path related global
:: variable with relative names
set CURRENT_DIR=%cd%
set BUILD_DIR=%CURRENT_DIR%\build
set REPO_DIR=%BUILD_DIR%\repo
set TARGET_DIR=%BUILD_DIR%\target
set RESOURCES_DIR=%TARGET_DIR%\resources
set SETUP_DIR=%TARGET_DIR%\setup
set BASE_DIR=%REPO_DIR%
set SRC_DIR=%BASE_DIR%\bin\hive_viriatum\i386\win32\Release
set SOLUTION_DIR=%BASE_DIR%\win32\vs2008ex

:: creates the various directories that are going to be
:: used during the build process
mkdir %BUILD_DIR%
mkdir %REPO_DIR%
mkdir %TARGET_DIR%
mkdir %RESOURCES_DIR%
mkdir %SETUP_DIR%

:: moves the current working directory to the build directory
:: so that all the generated files are placed there
cd %BUILD_DIR%

:: clones the repository to retrieve the source code
:: for compilation
git clone git://github.com/hivesolutions/viriatum.git %REPO_DIR%

:: runs the build process for the viriatum project, this
:: will lauch the build utility for it
msbuild %SOLUTION_DIR%\hive_viriatum.sln /p:Configuration=Release

:: changes the directory in order to group the files and then
:: returns tho the "original" build directory
cd %SRC_DIR%
tar -cf %NAME%.tar index.html viriatum.exe resources templates
xcopy /q /e /k index.html %RESOURCES_DIR%
xcopy /q /e /k viriatum.exe %RESOURCES_DIR%
xcopy /q /a /e /k resources %RESOURCES_DIR%\resources
xcopy /q /a /e /k templates %RESOURCES_DIR%\templates
xcopy /q /e /k %NAME%.tar %RESOURCES_DIR%
cd %BUILD_DIR%

echo Building capsule setup package...

:: runs the capsule process adding the viriatum group file
:: to it in order to create the proper intaller
capsule clone %SETUP_DIR%\%NAME%.exe
capsule extend %SETUP_DIR%\%NAME%.exe Viriatum "Viriatum HTTP Server" %RESOURCES_DIR%\%NAME%.tar

:: moves back to the current directory (back to the base)
cd %CURRENT_DIR%
