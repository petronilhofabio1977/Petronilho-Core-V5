#!/bin/bash
# PETRONILHO CORE V5 - Linux Build Script
# Version: 2.0.0 | Author: Fabio Petronilho de Oliveira

GREEN='\033[0;32m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}=== PETRONILHO CORE V5: LINUX BUILD ===${NC}"

# 1. Verifica dependencias
echo -e "1. Verificando dependencias..."

if ! command -v g++ &> /dev/null; then
    echo -e "${RED}ERRO: g++ nao encontrado.${NC}"
    echo -e "${YELLOW}Instale: sudo pacman -S gcc  (Arch)${NC}"
    echo -e "${YELLOW}         sudo apt install g++ (Ubuntu)${NC}"
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}ERRO: cmake nao encontrado.${NC}"
    echo -e "${YELLOW}Instale: sudo pacman -S cmake  (Arch)${NC}"
    echo -e "${YELLOW}         sudo apt install cmake (Ubuntu)${NC}"
    exit 1
fi

echo -e "${GREEN}   g++   : $(which g++)${NC}"
echo -e "${GREEN}   cmake : $(which cmake)${NC}"

# 2. Limpa build anterior
echo -e "2. Limpando build anterior..."
rm -rf build/
mkdir build

# 3. Configura com CMake
echo -e "3. Configurando CMake..."
cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-O3 -march=native"

if [ $? -ne 0 ]; then
    echo -e "${RED}ERRO: CMake falhou.${NC}"
    cd ..
    exit 1
fi

# 4. Compila
echo -e "4. Compilando..."
cmake --build . --parallel $(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}ERRO: Compilacao falhou.${NC}"
    cd ..
    exit 1
fi

cd ..

# 5. Governance check
echo -e "5. Verificando governanca..."
if command -v python3 &> /dev/null; then
    python3 scripts/governance_check.py .
else
    echo -e "${YELLOW}   Python3 nao encontrado, pulando governance check.${NC}"
fi

# 6. Benchmark com prioridade real-time
echo -e "6. Executando benchmark..."
if [ -f "build/research_suite" ]; then
    sudo chrt -f 99 taskset -c 2 ./build/research_suite
else
    echo -e "${YELLOW}   research_suite nao encontrado no build.${NC}"
fi

echo ""
echo -e "${GREEN}=== BUILD CONCLUIDO ===${NC}"
echo -e "${CYAN}Binarios em: ./build/${NC}"
echo -e "${CYAN}Para limpar: rm -rf build/${NC}"
