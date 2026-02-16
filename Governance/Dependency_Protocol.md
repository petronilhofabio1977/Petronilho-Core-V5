# SUPER_CORE Dependency Protocol v2.0

## 1. Hierarquia de Inclusão (Camadas)
- **L0 (CORE):** Zero dependências externas. C++23 puro.
- **L1 (WINGS):** Podem incluir `core/`.
- **L2 (RELATIONAL):** Pode incluir `core/` e `priority_wing/`.

## 2. Regras Absolutas
- Proibido `#include <vector>`, `<map>`, `<iostream>` em código de runtime.
- Proibido uso de caminhos relativos (`../`). Use caminhos da raiz.
- Todo header deve ser `alignas(64)` se for uma estrutura de dados de Arena.
- O uso de `virtual` ou `exceptions` resulta em falha imediata de auditoria.

## 3. Alinhamento de Cache
- Estruturas que residem na Arena devem ser múltiplos de 64 bytes para evitar 'False Sharing'.