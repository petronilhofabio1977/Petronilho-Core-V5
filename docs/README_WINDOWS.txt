===========================================================
   INSTRUCOES DE TESTE - PETRONILHO CORE V5 (WINDOWS)
===========================================================

Este projeto e cross-platform e funciona nativamente no Windows.
Para rodar a simulacao de alta performance, siga estes passos:

1. REQUISITOS:
   - Ter o compilador g++ instalado (MinGW ou MSYS2).
   - Abrir o PowerShell na pasta do projeto.

2. COMO EXECUTAR O TESTE AUTOMATICO:
   No PowerShell, digite:
   .\run_test.ps1

   (Nota: Se o Windows bloquear o script, rode este comando antes:
   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass)

3. O QUE SERA VALIDADO:
   - Compilacao dos binarios (.exe).
   - Alocacao de memoria via Win32 API (CreateFileMapping).
   - Persistencia de dados com Magic Number (0xDEADBEEF).
   - Visualizacao do DNA do arquivo via Hexadecimal.

4. COMANDOS MANUAIS (Caso nao queira usar o script):
   - Compilar: g++ -O3 main_universal.cpp -o petronilho_universal.exe
   - Rodar: .\petronilho_universal.exe
   - Ver Log: Format-Hex -Path .\universal_audit.log | Select-Object -First 10

-----------------------------------------------------------
Desenvolvido por: Fábio Petronilho de Oliveira (Eng. Comp. UNIVESP)
Foco: Ultra-Low Latency & High Performance Persistence
===========================================================
