@echo off
for %%i in (*.png) do (
  echo Compacting %%i
  PngReWrite %%i %%i
)
echo Done