@echo off

set Target=%1

if "%Target%" == "" (
    set Target="Makefile"
) else (
    if "%Target%" == "All" (
        set Target="Makefile"
    ) else (
        set Target="%Target%.make"
    )
)

cd build
mingw32-make.exe config=release -f %Target%