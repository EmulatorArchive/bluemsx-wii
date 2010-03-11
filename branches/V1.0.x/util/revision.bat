@echo off
cd %~p0
cscript //NoLogo //E:jscript svnversion.js >..\build\revision.tmp
cd ..\build
if exist revision.txt set /p revision_old= <revision.txt >nul
if exist revision.tmp set /p revision_new= <revision.tmp >nul
if not "%revision_old%"=="%revision_new%" (
  echo Revision set to %revision_new%
  copy /y revision.tmp revision.txt >nul
  echo #define REVISION_NUMBER %revision_new% >revision.inc
  echo #define REVISION_NUMBER_STRING "%revision_new%" >>revision.inc
)
del revision.tmp
set revision_old=
set revision_new=
