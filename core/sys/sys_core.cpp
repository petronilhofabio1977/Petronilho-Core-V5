// Layer: L1 | Version: 1.3.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// sys_core.cpp - Inicializacao do Subsistema Core
// ================================================================
//
// ALGORITMOS DO CORMEN UTILIZADOS:
//
// 1. ARVORE DE DECISAO - Cormen Cap.12 (Binary Search Trees)
//    Usado em: boot_info() selecao de ISA
//    A sequencia if/else if para ISA segue uma arvore de
//    decisao onde cada no e uma comparacao. Ordem importa:
//    do mais capaz (AVX512) para o menos (GENERIC).
//    Equivalente a busca em BST do maior para o menor.
//    Complexidade: O(h) onde h = altura da arvore = 4 nos
//    Na pratica O(1) pois h e fixo e pequeno.
//
// 2. BUSCA LINEAR - Cormen Cap.2 Sec.2.1 (Linear Search)
//    Usado em: strlen() dentro de write_msg()
//    strlen percorre o array de chars ate encontrar '\0'.
//    Cormen Cap.2 prova que busca linear e O(n) onde
//    n = tamanho da string. Inevitavel para strings C.
//    Complexidade: O(n) onde n = tamanho da mensagem
//
// PORTABILIDADE:
// write_msg usa WriteFile no Windows e write() no Linux.
// Sem iostream em nenhuma plataforma.
// ================================================================

#include "core/sys/arena_thread_local.hpp"
#include "core/sys/sys_core.hpp"
#include "core/sys/handle.hpp"
#include "core/platform/cpu_dispatch.hpp"
#include "core/platform/platform_detect.hpp"
#include <cstring>

#ifdef PETRONILHO_OS_WINDOWS
    #include <windows.h>
#else
    #include <unistd.h>
#endif

namespace petronilho::sys {

    // ============================================================
    // write_msg - Escrita direta sem iostream
    // ALGORITMO: Busca Linear - Cormen Cap.2 Sec.2.1
    // strlen() e O(n) onde n = tamanho da string.
    // Windows: WriteFile via handle de stderr
    // Linux:   write() direto no STDERR_FILENO
    // ============================================================
    static void write_msg(const char* msg) noexcept {
        if (!msg) return;
        const size_t len = strlen(msg); // O(n) Cormen Cap.2

    #ifdef PETRONILHO_OS_WINDOWS
        HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
        if (h && h != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            WriteFile(h,
                      msg,
                      static_cast<DWORD>(len),
                      &written,
                      nullptr);
        }
    #else
        ::write(STDERR_FILENO, msg, len);
    #endif
    }

    // ============================================================
    // boot_info - Exibe informacoes de hardware no boot
    //
    // ALGORITMO: Arvore de Decisao - Cormen Cap.12
    // Selecao de ISA segue arvore com 4 nos em altura fixa.
    // Ordem: AVX512 > AVX2 > SSE4 > GENERIC
    // Complexidade: O(1) pois arvore tem altura constante 4
    // ============================================================
    void boot_info() noexcept {
        // Cormen Cap.12: detect_isa() percorre arvore de decisao
        const auto isa     = petronilho::platform::detect_isa();
        const auto isa_str = petronilho::platform::isa_name(isa);

        write_msg("==============================================\n");
        write_msg("[PETRONILHO CORE] Sincronizando hardware...\n");
        write_msg("[PETRONILHO CORE] OS      : ");
        write_msg(PETRONILHO_OS_NAME);
        write_msg("\n");
        write_msg("[PETRONILHO CORE] Arch    : ");
        write_msg(PETRONILHO_ARCH_NAME);
        write_msg("\n");
        write_msg("[PETRONILHO CORE] ISA     : ");
        write_msg(isa_str);
        write_msg("\n");

        // Arvore de decisao O(1) altura fixa
        // Cormen Cap.12: cada no e uma comparacao de ISA
        if (isa == petronilho::platform::ISA::AVX512)
            write_msg("[CORE] STATUS: AVX512 ATIVO.\n");
        else if (isa == petronilho::platform::ISA::AVX2)
            write_msg("[CORE] STATUS: AVX2 ATIVO.\n");
        else if (isa == petronilho::platform::ISA::SSE4)
            write_msg("[CORE] STATUS: SSE4.2 (i7-620M) ATIVO.\n");
        else
            write_msg("[CORE] STATUS: GENERIC (sem SIMD).\n");

        write_msg("==============================================\n");
    }

} // namespace petronilho::sys