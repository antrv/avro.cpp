@echo off

set CONFIG=%1
IF "%1"=="" (
  set CONFIG=Debug
)

set ROOT=%~dp0
set WINSDKROOT=C:\Program Files (x86)\Windows Kits\10
set WINSDKVER=10.0.18362.0
set WINSDK_INCLUDE=%WINSDKROOT%\Include\%WINSDKVER%
set WINSDK_LIB=%WINSDKROOT%\Lib\%WINSDKVER%
set TOOLSET=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.22.27905
set CXX_COMPILER=%TOOLSET%\bin\Hostx64\x64\cl.exe
set OUTPUT_PATH=.bin\%CONFIG%
set IMMEDIATE_PATH=.tmp\%CONFIG%

if not exist %IMMEDIATE_PATH% (
  mkdir %IMMEDIATE_PATH%
)

if not exist %OUTPUT_PATH% (
  mkdir %OUTPUT_PATH%
)

rem Various
set CL=/utf-8 /Y- /W4 /permissive- /FC
rem Architecture
set CL=%CL% /Oi /arch:AVX2 /Gv /fp:precise
rem Exceptions
set CL=%CXX_FLAGS% /EHa
rem Compiling
set CL=%CL% /std:c++17 /TP /Gm-
rem Assembly listing
set CL=%CL% /Fa%IMMEDIATE_PATH%/app.asm /FAsu
rem Includes
set CL=%CL% "/I%TOOLSET%\include" "/I%WINSDK_INCLUDE%\ucrt" "/I.\include"
rem Output
set CL=%CL% /Fe%OUTPUT_PATH%\app.exe /Fd%OUTPUT_PATH%\app.pdb /Fo%IMMEDIATE_PATH%\app.obj /Fm%OUTPUT_PATH%\app.map 

rem Linker
set LINK=/DEBUG /MACHINE:X64 /SUBSYSTEM:CONSOLE /NXCOMPAT /DYNAMICBASE /INCREMENTAL:NO /TLBID:1
rem Libraries
set LIB=%TOOLSET%\lib\x64;%WINSDK_LIB%\ucrt\x64;%WINSDK_LIB%\um\x64

if "%CONFIG%"=="Debug" (
  set CL=%CL% /MDd /Od /JMC /Zi
  set LINK=%LINK%
)
if "%CONFIG%"=="Release" (
  set CL=%CL% /O2 /Oi /Gw /Gy /GL /MD /Zc:inline /Zi
  set LINK=%LINK% /RELEASE /OPT:REF /OPT:ICF
)

rem Internal sources
set SOURCES=
rem Library sources
rem set SOURCES=%SOURCES% src/bigint.cpp
rem Main sources
set SOURCES=%SOURCES% src/main.cpp

"%CXX_COMPILER%" /nologo %SOURCES%
