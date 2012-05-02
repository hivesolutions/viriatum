:: turns off the echo
@echo off

:: sets the various path related global
:: variable with relative names
set CURRENT_DIR=%cd%
set BUILD_DIR=%CURRENT_DIR%\build
set REPO_DIR=%BUILD_DIR%\repo
set TARGET_DIR=%BUILD_DIR%\target
set BASE_DIR=%REPO_DIR%
set SRC_DIR=%BASE_DIR%\bin\hive_viriatum\i386\win32\Release
set SOLUTION_DIR=%BASE_DIR%\win32\vs2008ex

:: sets the version string constant value
set VERSION=0.1.0

:: creates the various directories that are going to be
:: used during the build process
mkdir %BUILD_DIR%
mkdir %REPO_DIR%
mkdir %TARGET_DIR%

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
tar -cf viriatum_%VERSION%.tar index.html viriatum.exe resources templates
cd %BUILD_DIR%

:: runs the capsule process adding the viriatum group file
:: to it in order to create the proper intaller
capsule clone %TARGET_DIR%\viriatum-%VERSION%.exe
capsule extend %TARGET_DIR%\viriatum-%VERSION%.exe Viriatum "Viriatum HTTP Server" %SRC_DIR%\viriatum_%VERSION%.tar

:: moves back to the current directory (back to the base)
cd %CURRENT_DIR%
