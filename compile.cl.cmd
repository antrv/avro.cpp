@echo off
setlocal enabledelayedexpansion

set CONFIG=%1
IF "%1"=="" (
  set CONFIG=Debug
)

set ROOT=%~dp0
set OUTPUT_PATH=.bin\%CONFIG%
set IMMEDIATE_PATH=.tmp\%CONFIG%
if not exist %IMMEDIATE_PATH% (
  mkdir %IMMEDIATE_PATH%
)

if not exist %OUTPUT_PATH% (
  mkdir %OUTPUT_PATH%
)

rem Get VS installation folder
set VSWHEREPATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
for /F "tokens=* USEBACKQ" %%F in (`"%VSWHEREPATH%" -latest -property installationPath`) do (
  set VSROOTPATH=%%F
)

rem Get VC tools installation folder and cl.exe path
set /p VCTOOLSVERSION=<"%VSROOTPATH%\VC\Auxiliary\Build\Microsoft.VCToolsVersion.default.txt"
set VCTOOLSPATH=%VSROOTPATH%\VC\Tools\MSVC\%VCTOOLSVERSION%
set CXX_COMPILER=%VCTOOLSPATH%\bin\Hostx64\x64\cl.exe

rem Get Windows 10 SDK installation folder
for /F "tokens=1,2*" %%i in ('reg query "HKLM\SOFTWARE\Wow6432Node\Microsoft\Microsoft SDKs\Windows\v10.0" /v "InstallationFolder"') DO (
    if "%%i"=="InstallationFolder" (
        SET WINSDKROOT=%%~k
    )
)
for /F "tokens=1,2*" %%i in ('reg query "HKLM\SOFTWARE\Wow6432Node\Microsoft\Microsoft SDKs\Windows\v10.0" /v "ProductVersion"') DO (
    if "%%i"=="ProductVersion" (
        SET WINSDKVER=%%~k
    )
)

if "%WINSDKROOT%"=="" (
  for /F "tokens=1,2*" %%i in ('reg query "HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v10.0" /v "InstallationFolder"') DO (
      if "%%i"=="InstallationFolder" (
          SET WINSDKROOT=%%~k
      )
  )
  for /F "tokens=1,2*" %%i in ('reg query "HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v10.0" /v "ProductVersion"') DO (
      if "%%i"=="ProductVersion" (
          SET WINSDKVER=%%~k
      )
  )
)

set WINSDK_INCLUDE=%WINSDKROOT%Include\%WINSDKVER%.0
set WINSDK_LIB=%WINSDKROOT%Lib\%WINSDKVER%.0

rem Various
set CL=/utf-8 /Y- /W4 /permissive- /FC
rem Architecture
set CL=%CL% /Oi /arch:AVX2 /Gv /fp:precise
rem Exceptions
set CL=%CXX_FLAGS% /EHa
rem Compiling
set CL=%CL% /std:c++17 /TP /Gm- /Zi
rem Assembly listing
set CL=%CL% /Fa%IMMEDIATE_PATH%/app.asm /FAsu
rem Includes
set CL=%CL% "/I%VCTOOLSPATH%\include" "/I%WINSDK_INCLUDE%\ucrt" "/I.\include"
rem Output
set CL=%CL% /Fe%OUTPUT_PATH%\app.exe /Fd%OUTPUT_PATH%\app.pdb /Fo%IMMEDIATE_PATH%\app.obj /Fm%OUTPUT_PATH%\app.map 

rem Linker
set LINK=/DEBUG /MACHINE:X64 /SUBSYSTEM:CONSOLE /NXCOMPAT /DYNAMICBASE /INCREMENTAL:NO /TLBID:1
rem Libraries
set LIB=%VCTOOLSPATH%\lib\x64;%WINSDK_LIB%\ucrt\x64;%WINSDK_LIB%\um\x64

if "%CONFIG%"=="Debug" (
  set CL=%CL% /MDd /Od /JMC
  set LINK=%LINK%
)
if "%CONFIG%"=="Release" (
  set CL=%CL% /O2 /Gw /Gy /GL /MD /Zc:inline
  set LINK=%LINK% /RELEASE /OPT:REF /OPT:ICF
)

rem Sources
set SOURCES=%SOURCES% src/main.cpp

rem Compile
"%CXX_COMPILER%" /nologo %SOURCES%
