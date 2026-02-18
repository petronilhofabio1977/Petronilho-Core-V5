# PETRONILHO CORE V5 - WINDOWS AUTOMATION SCRIPT
Clear-Host
Write-Host "=== PETRONILHO CORE V5: WINDOWS DASHBOARD ===" -ForegroundColor Blue

# 1. Limpeza de arquivos antigos
Write-Host "1. Limpando arquivos antigos..." -ForegroundColor Gray
if (Test-Path "petronilho_universal.exe") { Remove-Item "petronilho_universal.exe" }
if (Test-Path "reader_universal.exe") { Remove-Item "reader_universal.exe" }
if (Test-Path "universal_audit.log") { Remove-Item "universal_audit.log" }

# 2. Compilação (Requer MinGW/GCC instalado no Windows)
Write-Host "2. Compilando binarios de alta performance..." -ForegroundColor Gray
g++ -O3 main_universal.cpp -o petronilho_universal.exe
g++ -O3 reader.cpp -o reader_universal.exe

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERRO: O g++ nao foi encontrado ou falhou ao compilar." -ForegroundColor Red
    exit
}

# 3. Execução do Ingestor
Write-Host "3. Executando Ingestor Universal..." -ForegroundColor Gray
.\petronilho_universal.exe

# 4. Visualização dos Dados (Dashboard)
Write-Host "4. Extraindo dados do log..." -ForegroundColor Gray
.\reader_universal.exe

# 5. Inspeção Hexadecimal (O "DNA" do arquivo no Windows)
Write-Host "`n>> INSPECAO HEXADECIMAL (DNA DO ARQUIVO) <<" -ForegroundColor Cyan
Format-Hex -Path .\universal_audit.log | Select-Object -First 10

Write-Host "`n=== TESTE CONCLUIDO COM SUCESSO NO WINDOWS ===" -ForegroundColor Green
Write-Host "Para limpar tudo, digite: rm *.log, *.exe" -ForegroundColor Yellow