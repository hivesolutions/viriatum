:: turns off the echo
@echo off

:: sets the version string constant value
set VERSION=0.1.0

:: sets the various global related name values
:: (going to be used for file construction)
set ARCHITECTURE=x86
set NAME=viriatum-%VERSION%

:: sets the directory to be used as the base
:: for the retrieval of the development tools
if not defined DEV_HOME set DEV_HOME=\dev

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
:: for compilation, the command is run using the call
:: operator so that it avoid exiting the current process
call git clone git://github.com/hivesolutions/viriatum.git %REPO_DIR% --quiet

:: in case the previous command didn't exit properly
:: must return immediately with the error
if %ERRORLEVEL% neq 0 ( cd %CURRENT_DIR% && exit /b %ERRORLEVEL% )

:: sets the proper include and lib directory for build then
:: runs the build process for the viriatum project, this
:: will lauch the build utility for it
set INCLUDE=%INCLUDE%;%DEV_HOME%\include
set LIB=%LIB%;%DEV_HOME%\lib
set PATH=%PATH%;%DEV_HOME%\bin;%DEV_HOME%\util
msbuild %SOLUTION_DIR%\hive_viriatum.sln /p:Configuration=Release /p:"VCBuildAdditionalOptions=/useenv"

:: in case the previous command didn't exit properly
:: must return immediately with the error
if %ERRORLEVEL% neq 0 ( cd %CURRENT_DIR% && exit /b %ERRORLEVEL% )

:: changes the directory in order to group the files and then
:: returns the "original" build directory
cd %SRC_DIR%
xcopy /q /y /e /k viriatum.exe %RESOURCES_DIR%
xcopy /q /y /a /e /k config %RESOURCES_DIR%\config
xcopy /q /y /a /e /k htdocs %RESOURCES_DIR%\htdocs
cd %RESOURCES_DIR%
tar -cf %NAME%.tar viriatum.exe config htdocs
cd %BUILD_DIR%

echo Building capsule setup package...

:: runs the capsule process adding the viriatum group file
:: to it in order to create the proper intaller
capsule clone %SETUP_DIR%\%NAME%.exe
capsule extend %SETUP_DIR%\%NAME%.exe Viriatum "Viriatum HTTP Server" %RESOURCES_DIR%\%NAME%.tar

:: in case the previous command didn't exit properly
:: must return immediately with the error
if %ERRORLEVEL% neq 0 ( cd %CURRENT_DIR% && exit /b %ERRORLEVEL% )

:: moves back to the current directory (back to the base)
cd %CURRENT_DIR%
