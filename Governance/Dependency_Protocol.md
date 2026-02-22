# SUPER_CORE Dependency Protocol v2.1
# Autor: Fabio Petronilho de Oliveira
# Revisao: v2.1 - Nivel Industrial

---

## 1. Hierarquia de Inclusao (Camadas)

| Camada | Nome       | Dependencias permitidas         |
|--------|------------|---------------------------------|
| L0     | CORE       | Zero. C++23 puro apenas.        |
| L1     | WINGS      | Pode incluir core/              |
| L2     | RELATIONAL | Pode incluir core/ e wings/     |
| L3     | APPLICATION| Pode incluir tudo               |

Todo arquivo deve declarar sua camada na primeira linha:
```cpp
// Layer: L0 | Version: 1.0.0 | Author: Fabio Petronilho | CLRS Cap.32
```

---

## 2. Regras Absolutas de Runtime (L0 e L1)

### Proibido em L0 e L1:
- `#include <vector>`
- `#include <map>`
- `#include <unordered_map>`
- `#include <iostream>`
- `#include <string>`
- `#include <fstream>`
- Uso de `virtual`
- Uso de `throw` ou `try/catch`
- Uso de `new` ou `delete`
- Caminhos relativos com `../`

### Permitido em L0:
- `#include <cstdint>`
- `#include <cstring>`
- `#include <atomic>`
- `#include <type_traits>`
- `#include <array>`
- `#include <span>`
- Headers do sistema: `<sys/mman.h>`, `<fcntl.h>`, `<unistd.h>`

### Permitido em L1:
- Tudo de L0
- `#include <chrono>`
- `#include <thread>`

---

## 3. Alinhamento de Cache

- Estruturas que residem na Arena devem usar `alignas(64)`
- Tamanho total deve ser multiplo de 64 bytes
- Usar `static_assert(sizeof(T) % 64 == 0)` para verificar em compilacao
- Separar dados de leitura e escrita em cache lines diferentes
  para evitar False Sharing em contexto multithread

```cpp
// Correto
struct alignas(64) MinhaEstrutura {
    uint64_t campo_a;  // 8 bytes
    uint64_t campo_b;  // 8 bytes
    uint8_t  _pad[48]; // 48 bytes padding
};                     // total = 64 bytes

static_assert(sizeof(MinhaEstrutura) == 64);
```

---

## 4. Nomenclatura

| Elemento     | Padrao           | Exemplo                    |
|--------------|------------------|----------------------------|
| Arquivos     | snake_case.hpp   | ring_buffer.hpp            |
| Classes      | PascalCase       | RingBuffer                 |
| Metodos      | snake_case()     | write(), read()            |
| Constantes   | UPPER_SNAKE_CASE | MAX_SLOTS                  |
| Variaveis    | snake_case       | m_write_idx                |
| Namespaces   | lowercase        | petronilho::               |
| Membros      | m_ prefix        | m_capacity                 |
| Estaticos    | s_ prefix        | s_instance                 |

---

## 5. Documentacao Obrigatoria

Todo componente L0 deve documentar:

```cpp
/*
 * NomeDoComponente
 * Algoritmo: Nome do algoritmo utilizado
 * Base teorica: Cormen et al. CLRS, Capitulo XX
 * Complexidade Tempo: O(?)
 * Complexidade Espaco: O(?)
 * Thread-safety: sim/nao
 * Excecoes: nenhuma (proibido em L0)
 */
```

---

## 6. Testes

- Todo componente L0 deve ter arquivo correspondente em `tests/`
- Nomenclatura: `test_nome_do_componente.cpp`
- Deve compilar e executar sem erros ou warnings
- Deve verificar pelo menos: caso normal, caso vazio, caso cheio

---

## 7. Versioning

Formato: `MAJOR.MINOR.PATCH`

- MAJOR: mudanca incompativel de interface
- MINOR: nova funcionalidade compativel
- PATCH: correcao de bug

---

## 8. Classificacao dos Arquivos Atuais

| Arquivo              | Camada Correta | Observacao                          |
|----------------------|----------------|-------------------------------------|
| ring_buffer.hpp      | L0             | Conforme                            |
| protocol_decoder.hpp | L0             | Conforme                            |
| super_core.hpp       | L0             | Conforme                            |
| main.cpp             | L3             | Usa iostream, fstream, chrono       |
| bench_*.cpp          | L3             | Arquivos de teste e benchmark       |
