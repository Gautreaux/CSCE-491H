REM basically build/test in subdir and then copy to current dir
@echo off
echo --------- Starting Build Process ------------

REM Move into the sub directory where the cpp files and stuff are located
cd cpp_impl
REM call make with added debug information if present
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

REM TODO - this is a good place where tests could be run if needed

REM COPY the EXE from the sub directory to the main one
set movePath=%CD%\cpp_impl\Main.exe 
MOVE /Y %movePath%

set moveExit=%ERRORLEVEL%
if %moveExit% NEQ 0 (
    ECHO Move Failed with exit code %moveExit%
    ECHO For reference CD is: %CD%
    ECHO Target path was: %movePath%
    exit /B %moveExit%
)

ECHO %date% %time%
ECHO ------------ Build  Sucessful ---------------
exit /B 0