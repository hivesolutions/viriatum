:: turns off the echo
@echo off

:: changes the current directory to the base one
:: and runs the file gathering
cd ../../../
python scripts/util/all/file_gathering.py
