# PETRONILHO CORE V5 - Windows Build Script
# Version: 2.0.0 | Author: Fabio Petronilho de Oliveira

Clear-Host
Write-Host "=== PETRONILHO CORE V5: WINDOWS BUILD ===" -ForegroundColor Blue

# 1. Verifica dependencias
Write-Host "1. Verificando dependencias..." -ForegroundColor Gray

$gcc = Get-Command g++ -ErrorAction SilentlyContinue
$cmake = Get-Command cmake -ErrorAction SilentlyContinue

if (-not $gcc) {
    Write-Host "ERRO: g++ nao encontrado. Instale MinGW-w64." -ForegroundColor Red
    Write-Host "Download: https://winlibs.com" -ForegroundColor Yellow
    exit 1
}

if (-not $cmake) {
    Write-Host "ERRO: cmake nao encontrado. Instale CMake." -ForegroundColor Red
    Write-Host "Download: https://cmake.org/download" -ForegroundColor Yellow
    exit 1
}

Write-Host "   g++   : $($gcc.Source)" -ForegroundColor Green
Write-Host "   cmake : $($cmake.Source)" -ForegroundColor Green

# 2. Limpa build anterior
Write-Host "2. Limpando build anterior..." -ForegroundColor Gray
if (Test-Path "build") { Remove-Item -Recurse -Force "build" }
New-Item -ItemType Directory -Path "build" | Out-Null

# 3. Configura com CMake
Write-Host "3. Configurando CMake..." -ForegroundColor Gray
Push-Location "build"

cmake .. -G "MinGW Makefiles" `
         -DCMAKE_BUILD_TYPE=Release `
         -DCMAKE_CXX_FLAGS="-O3 -march=native"

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERRO: CMake falhou." -ForegroundColor Red
    Pop-Location
    exit 1
}

# 4. Compila
Write-Host "4. Compilando..." -ForegroundColor Gray
cmake --build . --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERRO: Compilacao falhou." -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location

# 5. Executa governance check
Write-Host "5. Verificando governanca..." -ForegroundColor Gray
$python = Get-Command python -ErrorAction SilentlyContinue
if ($python) {
    python scripts/governance_check.py .
} else {
    Write-Host "   Python nao encontrado, pulando governance check." -ForegroundColor Yellow
}

# 6. Executa research_suite
Write-Host "6. Executando benchmark..." -ForegroundColor Gray
if (Test-Path "build\research_suite.exe") {
    .\build\research_suite.exe
} else {
    Write-Host "   research_suite nao encontrado no build." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=== BUILD CONCLUIDO ===" -ForegroundColor Green
Write-Host "Binarios em: .\build\" -ForegroundColor Cyan
