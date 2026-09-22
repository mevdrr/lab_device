@echo off
setlocal
if not exist build\lab_demo.exe (
  echo Build first with CMake. See README.md.
  exit /b 1
)
build\lab_demo.exe
