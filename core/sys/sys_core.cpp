// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// sys_core.cpp - Inicializacao do Subsistema Core
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Inicializa o subsistema sys do Core: detecta hardware,
// exibe informacoes de boot e prepara o ambiente de execucao.
// E o ponto de entrada do modulo sys.
//
// CAMADA L1: usa platform/ (L0) e sys/ (L0)
// Permitido usar chrono e thread em L1.
// cout proibido: substituido por write() direto em stderr
// para evitar dependencia de iostream em runtime critico.
// ================================================================

#include "core/sys/arena_thread_local.hpp"
#include "core/sys/sys_core.hpp"
#include "core/sys/handle.hpp"
#include "core/platform/cpu_dispatch.hpp"
#include "core/platform/platform_detect.hpp"
#include <cstring>
#include <unistd.h>

namespace petronilho::sys {

    // ============================================================
    // write_msg - Escrita direta sem iostream
    // Substituicao de std::cout proibido em L1 runtime critico
    // ============================================================
    static void write_msg(const char* msg) noexcept {
        ::write(STDERR_FILENO, msg, strlen(msg));
    }

    // ============================================================
    // boot_info - Exibe informacoes de hardware no boot
    // Detecta ISA via cpu_dispatch e registra no stderr
    // ============================================================
    void boot_info() noexcept {
        const auto isa      = petronilho::platform::detect_isa();
        const auto isa_str  = petronilho::platform::isa_name(isa);

        write_msg("==============================================\n");
        write_msg("[PETRONILHO CORE] Sincronizando hardware...\n");
        write_msg("[PETRONILHO CORE] ISA detectado: ");
        write_msg(isa_str);
        write_msg("\n");

        if (isa == petronilho::platform::ISA::SSE4)
            write_msg("[CORE] STATUS: SSE4.2 (i7-620M) ATIVO.\n");
        else if (isa == petronilho::platform::ISA::AVX2)
            write_msg("[CORE] STATUS: AVX2 ATIVO.\n");
        else if (isa == petronilho::platform::ISA::AVX512)
            write_msg("[CORE] STATUS: AVX512 ATIVO.\n");
        else
            write_msg("[CORE] STATUS: GENERIC (sem SIMD).\n");

        write_msg("==============================================\n");
    }

} // namespace petronilho::sys