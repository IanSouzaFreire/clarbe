@echo off
setlocal enabledelayedexpansion

set "CXX=D:\msys64\mingw64\bin\g++.exe"
set "FLAGS=-std=c++23 -O0 -g -Wall -Wextra -Wno-unused-parameter"
set "LIBS=-lzip -lspdlog -lfmt -ltomlplusplus -lboost_filesystem-mt -pthread"
set "INC=-I ..\include"

if not exist build mkdir build
cd build
if not exist bin mkdir bin

if not exist cmd mkdir cmd

echo Building command DLLs...
for %%f in (..\commands\*.cpp) do (
    echo Compiling %%~nf.cpp
    "%CXX%" -fPIC -c "%%f" -o "cmd\%%~nf.o" %INC% %LIBS% %FLAGS% %CFLAGS% %CXXFLAGS% %LDFLAGS%
    "%CXX%" -shared -o "bin\%%~nf.dll" -fPIC "cmd\%%~nf.o" %INC% %LIBS% %FLAGS% %CFLAGS% %CXXFLAGS% %LDFLAGS%
)

cd ..

endlocal