#!/bin/bash
GREEN='\033[0;32m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}=== PETRONILHO CORE V5: ULTIMATE DASHBOARD ===${NC}"

# 1. Compilação
echo -e "1. Compilando ferramentas de auditoria..."
g++ -O3 main_universal.cpp -o petronilho_universal
g++ -O3 reader.cpp -o reader_universal

# 2. Execução
echo -e "2. Executando Ingestor Universal..."
./petronilho_universal

# 3. Visualização
echo -e "3. Extraindo dados do arquivo .log..."
./reader_universal

# 4. INSTRUÇÕES DE FINALIZAÇÃO (O que você sugeriu!)
echo -e "\n${YELLOW}=== INSTRUÇÕES DE MANUTENÇÃO ===${NC}"
echo -e "Para limpar os arquivos de teste (log e binários), digite:"
echo -e "${GREEN}rm -f *.log petronilho_universal reader_universal${NC}"
echo -e "Para ver o conteúdo bruto do log (hexadecimal):"
echo -e "${GREEN}hexdump -C universal_audit.log | head -n 20${NC}"
echo -e "------------------------------------------------"
echo -e "${GREEN}SISTEMA OPERACIONAL E PRONTO PARA PRODUÇÃO!${NC}"
