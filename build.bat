@echo off
setlocal enabledelayedexpansion

set "CXX=D:\msys64\mingw64\bin\g++.exe"
set "FLAGS=-std=c++23 -O0 -g -Wall -Wextra -Wno-unused-parameter"
set "LIBS=-lzip -lspdlog -lfmt -ltomlplusplus -lboost_filesystem-mt -pthread"
set "INC=-I ..\include"

if not exist build mkdir build
cd build
if not exist bin mkdir bin

echo Compiling source files...
for %%f in (..\src\*.cpp) do (
    echo Compiling %%~nf.cpp
    "%CXX%" -c "%%f" -o "%%~nf.o" %INC% %LIBS% %FLAGS% %CFLAGS% %CXXFLAGS% %LDFLAGS%
)

echo Building binary:
"%CXX%" -o bin\clarbe.exe main.o %INC% %LIBS% %FLAGS% %CFLAGS% %CXXFLAGS% %LDFLAGS%

cd ..

endlocal