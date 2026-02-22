#!/usr/bin/env python3
"""
SUPER_CORE Governance Checker v1.0
Autor: Fabio Petronilho de Oliveira
Verifica se os arquivos estao em conformidade com GOVERNANCE.md
"""

import os
import sys
import re
from pathlib import Path

# Cores para terminal
RED   = "\033[1;31m"
GREEN = "\033[1;32m"
YELLOW= "\033[1;33m"
BLUE  = "\033[1;34m"
RESET = "\033[0m"

# Regras por camada
FORBIDDEN_L0 = [
    "#include <vector>",
    "#include <map>",
    "#include <unordered_map>",
    "#include <iostream>",
    "#include <string>",
    "#include <fstream>",
    "virtual ",
    "throw ",
    "try {",
    "catch(",
    " new ",
    " delete ",
    "../",
]

FORBIDDEN_L1 = FORBIDDEN_L0  # L1 herda restricoes de L0

# Arquivos e suas camadas esperadas
LAYER_MAP = {
    "ring_buffer.hpp":      "L0",
    "protocol_decoder.hpp": "L0",
    "super_core.hpp":       "L0",
    "arena.hpp":            "L0",
    "persistent_arena.hpp": "L1",
    "main.cpp":             "L3",
}

def check_file(filepath: Path, expected_layer: str) -> dict:
    results = {
        "file": filepath.name,
        "layer": expected_layer,
        "violations": [],
        "warnings": [],
        "passed": True,
    }

    try:
        content = filepath.read_text(encoding="utf-8", errors="ignore")
        lines   = content.splitlines()
    except Exception as e:
        results["violations"].append(f"Erro ao ler arquivo: {e}")
        results["passed"] = False
        return results

    # 1. Verificar declaracao de camada na primeira linha
    if lines and "Layer:" not in lines[0]:
        results["warnings"].append(
            "Primeira linha nao declara camada. "
            "Adicione: // Layer: L0 | Version: X.Y | Author: ..."
        )

    # 2. Verificar violacoes de includes e keywords proibidas
    forbidden = FORBIDDEN_L0 if expected_layer in ("L0", "L1") else []

    for i, line in enumerate(lines, 1):
        stripped = line.strip()
        if stripped.startswith("//"):
            continue  # ignora comentarios
        for rule in forbidden:
            if rule in line:
                results["violations"].append(
                    f"Linha {i}: PROIBIDO em {expected_layer} -> '{rule.strip()}'"
                )
                results["passed"] = False

    # 3. Verificar alinhamento de estruturas
    structs = re.findall(r'struct\s+(\w+)\s*\{', content)
    for struct in structs:
        if f"alignas(64)" not in content and expected_layer == "L0":
            results["warnings"].append(
                f"Estrutura '{struct}' pode precisar de alignas(64). "
                "Verifique se e uma estrutura de Arena."
            )
        # Verificar static_assert de tamanho
        if f"static_assert(sizeof({struct})" not in content:
            if "alignas(64)" in content:
                results["warnings"].append(
                    f"Estrutura '{struct}' com alignas(64) sem "
                    f"static_assert(sizeof({struct}) == 64)"
                )

    # 4. Verificar documentacao obrigatoria em L0
    if expected_layer == "L0":
        if "Algoritmo:" not in content:
            results["warnings"].append(
                "Documentacao L0 incompleta: falta 'Algoritmo:'"
            )
        if "Complexidade" not in content:
            results["warnings"].append(
                "Documentacao L0 incompleta: falta 'Complexidade'"
            )
        if "CLRS" not in content and "Cormen" not in content:
            results["warnings"].append(
                "Documentacao L0 incompleta: falta referencia ao CLRS/Cormen"
            )

    # 5. Verificar nomenclatura de membros
    bad_members = re.findall(r'\bpublic:\s*\w+\s+(?!m_|s_)(\w+)_', content)
    # Verificacao simplificada de prefixo m_
    private_vars = re.findall(r'private:.*?(?=public:|protected:|$)',
                               content, re.DOTALL)
    for block in private_vars:
        vars_found = re.findall(r'\b(?:int|uint\w+|size_t|bool|float|double)'
                                 r'\s+(?!m_)(\w+)\s*[;=]', block)
        for var in vars_found:
            if not var.startswith("m_") and not var.startswith("_"):
                results["warnings"].append(
                    f"Variavel privada '{var}' deveria ter prefixo 'm_'"
                )

    return results

def print_report(results: list):
    print(f"\n{BLUE}{'='*60}{RESET}")
    print(f"{BLUE}  SUPER_CORE Governance Checker v1.0{RESET}")
    print(f"{BLUE}{'='*60}{RESET}\n")

    total_files      = len(results)
    total_passed     = sum(1 for r in results if r["passed"])
    total_violations = sum(len(r["violations"]) for r in results)
    total_warnings   = sum(len(r["warnings"]) for r in results)

    for r in results:
        status = f"{GREEN}PASSOU{RESET}" if r["passed"] else f"{RED}FALHOU{RESET}"
        print(f"[{status}] {r['file']} (Camada: {r['layer']})")

        for v in r["violations"]:
            print(f"  {RED}VIOLACAO:{RESET} {v}")
        for w in r["warnings"]:
            print(f"  {YELLOW}AVISO:{RESET}    {w}")

        if not r["violations"] and not r["warnings"]:
            print(f"  {GREEN}Sem violacoes ou avisos.{RESET}")
        print()

    print(f"{BLUE}{'='*60}{RESET}")
    print(f"Arquivos verificados : {total_files}")
    print(f"Aprovados            : {GREEN}{total_passed}{RESET}")
    print(f"Reprovados           : {RED}{total_files - total_passed}{RESET}")
    print(f"Violacoes totais     : {RED}{total_violations}{RESET}")
    print(f"Avisos totais        : {YELLOW}{total_warnings}{RESET}")
    print(f"{BLUE}{'='*60}{RESET}\n")

    return total_violations == 0

def main():
    # Detecta a pasta do projeto
    project_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(".")

    print(f"Verificando pasta: {project_dir.resolve()}")

    results = []
    for filename, layer in LAYER_MAP.items():
        filepath = project_dir / filename
        if filepath.exists():
            results.append(check_file(filepath, layer))
        else:
            print(f"{YELLOW}AVISO: {filename} nao encontrado, pulando.{RESET}")

    if not results:
        print(f"{RED}Nenhum arquivo encontrado para verificar.{RESET}")
        sys.exit(1)

    passed = print_report(results)
    sys.exit(0 if passed else 1)

if __name__ == "__main__":
    main()
