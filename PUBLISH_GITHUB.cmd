@echo off
setlocal
git push -u origin variant-12-recycle
if errorlevel 1 exit /b 1
echo.
echo Branch published. Open this page to create the pull request:
echo https://github.com/mevdrr/lab_device/compare/main...variant-12-recycle
start "" "https://github.com/mevdrr/lab_device/compare/main...variant-12-recycle"
