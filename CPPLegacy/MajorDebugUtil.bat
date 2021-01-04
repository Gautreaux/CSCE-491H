@echo off
echo --------- Starting Build Process ------------

REM call make with added debug information if present
if "%~1" == "" (
    ECHO NO COMMAND LINE ARGS
    ECHO \n
    make
) else (
    ECHO COMMAND LINE ARGS FOUND: %*
    ECHO \n
    make %*
)

set makeExit=%ERRORLEVEL%
ECHO Make exited with code %makeExit%

del /S OutputHistory
mkdir OutputHistory

Main.exe > OutputHistory/test1.txt

set t1Exit=%ERRORLEVEL%
echo Trial 1 Exited with code %t1Exit%