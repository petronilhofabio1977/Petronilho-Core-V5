#!/bin/bash
# audit.sh - Versão Corrigida
echo "[AUDIT] Iniciando validação de protocolo em /core, /relational, /priority_wing e /geometric_wing..."
FAIL=0

# Lista de diretórios a auditar (ignora build, Governance e o próprio script)
TARGETS="core relational priority_wing geometric_wing"

# 1. Verificar caminhos relativos
if grep -r "\.\./" $TARGETS; then
    echo "[ERROR] Caminhos relativos detectados!"
    FAIL=1
fi

# 2. Verificar STL proibida
if grep -rE "std::vector|std::map|std::list" $TARGETS; then
    echo "[ERROR] Uso de STL dinâmica detectada!"
    FAIL=1
fi

# 3. Verificar Exceções/RTTI
if grep -rE "throw |catch |dynamic_cast" $TARGETS; then
    echo "[ERROR] Exceções ou RTTI detectados!"
    FAIL=1
fi

if [ $FAIL -eq 0 ]; then
    echo "[SUCCESS] Protocolo respeitado."
    exit 0
else
    echo "[FAIL] Auditoria falhou."
    exit 1
fi