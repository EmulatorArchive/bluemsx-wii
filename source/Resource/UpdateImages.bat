@echo off
SETLOCAL ENABLEEXTENSIONS
SETLOCAL ENABLEDELAYEDEXPANSION
goto main

:Generate
set image=%1
set directory_a=%2
set directory_b=%3

echo.>list.txt
if not "%directory_a%"=="" dir /b/s %directory_a% >>list.txt
if not "%directory_b%"=="" dir /b/s %directory_b% >>list.txt
SpriteSheetPacker\sspack.exe /image:%image%.png /map:%image%.txt /pad:1 /mw:2048 /pow2 /il:list.txt
del list.txt >nul

echo. >%image%_pos.inc
for /f "usebackq delims=" %%z in (%image%.txt) do (
  for /f "tokens=1,2,3,4,5,6 delims= " %%a in ("%%z") do (
    echo { "%%a", %%c, %%d, %%e, %%f }, >>%image%_pos.inc
  )
)
del %image%.txt >nul
exit /b 0

:main
call :Generate image_gui     BlueMsxWii
