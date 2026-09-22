@echo off
setlocal
if not exist build\lab_tests.exe (
  echo Build first with CMake. See README.md.
  exit /b 1
)
build\lab_tests.exe
