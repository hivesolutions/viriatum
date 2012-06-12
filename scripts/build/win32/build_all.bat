:: turns off the echo
@echo off

:: runs the build script and then renames the
:: build directory into base directory (this
:: should be the final name)
call build.bat
move build base

:: runs the build moduless script and then
:: renames the build directory into modules
:: directory (this should be the final name)
call build_modules.bat
move build modules
