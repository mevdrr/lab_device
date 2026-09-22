$ErrorActionPreference = 'Stop'
$repoDirectory = Split-Path $PSScriptRoot -Parent
$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmakeCommand) { throw 'CMake 3.20+ is required. Install it and add it to PATH.' }
if (-not (Get-Command g++ -ErrorAction SilentlyContinue)) { throw 'A C++17 compiler is required. Add MinGW g++ to PATH.' }
Push-Location $repoDirectory
try {
  & $cmakeCommand.Source -S . -B build-windows -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Debug
  if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
  & $cmakeCommand.Source --build build-windows --parallel 2
  if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
  & (Join-Path (Split-Path $cmakeCommand.Source -Parent) 'ctest.exe') --test-dir build-windows --output-on-failure --output-junit test-results.xml
  if ($LASTEXITCODE -ne 0) { throw 'Tests failed.' }
} finally { Pop-Location }
