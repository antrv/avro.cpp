@echo off
set PATH=%PATH%;%DEVDIR%\Tools\clang\bin

set CONFIG=%1
IF "%1"=="" (
  set CONFIG=Debug
)

if not exist .bin\%CONFIG% (
  mkdir .bin\%CONFIG%
)

set ROOT=%~dp0

rem Warnings
set CXX_FLAGS=-Wall
rem Architecture
set CXX_FLAGS=%CXX_FLAGS% -m64 -msse4.2 -mfpmath=sse
rem Compiling
set CXX_FLAGS=%CXX_FLAGS% -std=c++17 -I ./include -fno-ident -static
rem Assembly listing
set CXX_FLAGS=%CXX_FLAGS% -masm=intel --save-temps=obj
rem Clang specific
set CXX_FLAGS=%CXX_FLAGS% -Xclang -flto-visibility-public-std

if "%CONFIG%"=="Debug" (
  set CXX_FLAGS=%CXX_FLAGS% -g -gcodeview -gno-column-info
)
if "%CONFIG%"=="Release" (
  set CXX_FLAGS=%CXX_FLAGS% -O3
)

rem Internal sources
set SOURCES=
rem Library sources
rem set SOURCES=%SOURCES% src/bigint.cpp
rem Main sources
set SOURCES=%SOURCES% src/main.cpp 

rem clang++.exe %CXX_FLAGS% %SOURCES% -S -o .bin/%CONFIG%/app.asm
clang++.exe %CXX_FLAGS% %SOURCES% -o .bin/%CONFIG%/app.exe
