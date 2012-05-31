:: turns off the echo
@echo off

:: sets the version string constant value
set VERSION=0.1.0

:: sets the arquitecture for the current build
:: execution process (eg: win32, x64, etc.)
set ARCHITECTURE=win32

:: sets the various global related name values
:: (going to be used for file construction)
set NAME=viriatum-%VERSION%-%ARCHITECTURE%-modules
set NAME_RAW=viriatum-%VERSION%-%ARCHITECTURE%-modules-raw

:: sets the directory to be used as the base
:: for the retrieval of the development tools
if not defined DEV_HOME set DEV_HOME=\dev

:: sets the various path related global
:: variable with relative names
set CURRENT_DIR=%cd%
set BUILD_DIR=%CURRENT_DIR%\build
set REPO_DIR=%BUILD_DIR%\repo
set TARGET_DIR=%BUILD_DIR%\target
set TEMP_DIR=%TARGET_DIR%\tmp
set RESULT_DIR=%TARGET_DIR%\result
set DIST_DIR=%TARGET_DIR%\dist
set BASE_DIR=%REPO_DIR%
set SRC_DIRS=(%BASE_DIR%\bin\hive_viriatum_mod_lua\i386\win32\Release %BASE_DIR%\bin\hive_viriatum_mod_php\i386\win32\Release)
set SOLUTION_DIR=%BASE_DIR%\win32\vs2008ex

:: creates the various directories that are going to be
:: used during the build process
mkdir %BUILD_DIR%
mkdir %REPO_DIR%
mkdir %TARGET_DIR%
mkdir %TEMP_DIR%
mkdir %RESULT_DIR%
mkdir %DIST_DIR%

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

:: removes the internal git repository directory to avoid
:: extra files in source distribution
rmdir /q /s %REPO_DIR%\.git
del /q /d %REPO_DIR%\.gitignore

:: sets the proper include and lib directory for build then
:: runs the build process for the viriatum project, this
:: will lauch the build utility for it
set INCLUDE=%INCLUDE%;%DEV_HOME%\include;%DEV_HOME%\include\php;%DEV_HOME%\include\php\main;%DEV_HOME%\include\php\TSRM;%DEV_HOME%\include\php\Zend
set LIB=%LIB%;%DEV_HOME%\lib
set PATH=%PATH%;%DEV_HOME%\bin;%DEV_HOME%\util
msbuild %SOLUTION_DIR%\hive_viriatum_mod.sln /p:Configuration=Release /p:"VCBuildAdditionalOptions=/useenv"

:: in case the previous command didn't exit properly
:: must return immediately with the error
if %ERRORLEVEL% neq 0 ( cd %CURRENT_DIR% && exit /b %ERRORLEVEL% )

:: changes the directory in order to group the files and then
:: returns the "original" build directory
for %%S in %SRC_DIRS% do (
    cd %%S
    xcopy /q /y /e /k viriatum_*.dll %RESULT_DIR%\modules\
)
xcopy /q /y /a /e /k %RESULT_DIR% %TEMP_DIR%\%NAME%\
cd %RESULT_DIR%
tar -cf %NAME_RAW%.tar modules
move %NAME_RAW%.tar %DIST_DIR%
cd %TEMP_DIR%
zip -qr %NAME%.zip %NAME%
tar -cf %NAME%.tar %NAME%
gzip -c %NAME%.tar > %NAME%.tar.gz
move %NAME%.zip %DIST_DIR%
move %NAME%.tar %DIST_DIR%
move %NAME%.tar.gz %DIST_DIR%
cd %BUILD_DIR%

echo Building capsule setup package...

:: runs the capsule process adding the viriatum group file
:: to it in order to create the proper intaller
capsule clone %DIST_DIR%\%NAME%.exe
capsule extend %DIST_DIR%\%NAME%.exe Viriatum "Viriatum HTTP Server Modules" %DIST_DIR%\%NAME_RAW%.tar

:: in case the previous command didn't exit properly
:: must return immediately with the error
if %ERRORLEVEL% neq 0 ( cd %CURRENT_DIR% && exit /b %ERRORLEVEL% )

:: runs the checksums on the various elements of the setup
:: dir then copies the result into the setu directory
cd %DIST_DIR%
for %%F in (*) do (
    md5sum %%F > %TEMP_DIR%\%%F.md5
)
md5sum * > %TEMP_DIR%\MD5SUMS
move %TEMP_DIR%\*.md5 %DIST_DIR%
move %TEMP_DIR%\MD5SUMS %DIST_DIR%
cd %BUILD_DIR%

:: removes the directories that are no longer required
:: for the build
rmdir /q /s %TEMP_DIR%

:: moves back to the current directory (back to the base)
cd %CURRENT_DIR%
