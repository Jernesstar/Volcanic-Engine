@echo off

set componentPath=%1
cd /D %componentPath%
cd "Build\Platform\gcc-Windows"
mingw32-make.exe -f Makefile