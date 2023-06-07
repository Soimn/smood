@echo off

setlocal

cd %~dp0

if not exist build mkdir build
cd build

if "%Platform%" neq "x64" (
	echo ERROR: Platform is not "x64" - please run this from the MSVC x64 native tools command prompt.
	goto end
)

set "common_compile_options= /nologo /W3 /GS-"
set "common_link_options= /incremental:no /opt:ref /subsystem:windows libvcruntime.lib"


if "%1"=="debug" (
  set "compile_options=%common_compile_options% /Od /Zo /Z7 /RTC1 /MTd"
  set "link_options=%common_link_options% libucrtd.lib libvcruntimed.lib"
) else if "%1"=="release" (
  set "compile_options=%common_compile_options% /O2"
  set "link_options=%common_link_options%"
) else (
	goto end invalid_arguments
)

if "%2" neq "" goto invalid_arguments

cl %compile_options% ..\src\main.c /link %link_options% /pdb:smood.pdb /out:smood.exe Kernel32.lib User32.lib Gdi32.lib Winmm.lib Shlwapi.lib

goto end

:invalid_arguments
echo Invalid arguments^. Usage: build ^<debug^|release^>
goto end

:end
endlocal
