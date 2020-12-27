echo "Starting build process"
@echo off
cd cpp_impl
@echo on
make
@ echo off
cd ..
MOVE /Y "%CD%"\cpp_impl\dbgTst.exe 
@echo on

echo "Build OK"