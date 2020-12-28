REM basically just build/test in subdir and then copy to current dir
@echo off
echo --------- Starting Build Process ------------

cd cpp_impl
REM # call make with added debug information present
if "%~1" == "" (
    ECHO NO COMMMAND LINE ARGS
    ECHO \n
    make
) else (
    ECHO COMMAND LINE ARGS FOUND: %*
    ECHO \n
    make %*
)

set makeExit=%ERRORLEVEL%
ECHO Make exited with code %makeExit%

REM move back to the starting directory
cd ..

if %makeExit% NEQ 0 (
    ECHO Terminating build, last build was not modified
    exit /B %makeExit%
)

set movePath=%CD%\cpp_impl\Main.exe 
MOVE /Y %movePath%

set moveExit=%ERRORLEVEL%
if %moveExit% NEQ 0 (
    ECHO Move Failed with exit code %moveExit%
    ECHO For reference CD is: %CD%
    ECHO Target path was: %movePath%
    exit /B %moveExit%
)

ECHO ------------ Build  Sucessful ---------------

exit /B 0