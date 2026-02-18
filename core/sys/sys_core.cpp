#include "arena_thread_local.hpp"
#include "arena_thread_local.hpp"
#include "core/sys/sys_core.hpp"
#include "core/sys/handle.hpp"
#include "core/platform/cpu_dispatch.hpp"
#include <iostream>

namespace super_core::sys {

uint8_t* G_POOL_BASE = nullptr;

void boot_info() {
    auto isa = platform::detect_isa();
    std::cout << "==============================================" << std::endl;
    std::cout << "[CORE] Sincronizando hardware..." << std::endl;
    if (isa == platform::ISA::SSE4) 
        std::cout << "[CORE] STATUS: SSE4.2 (PETRONILHO) ATIVO." << std::endl;
    std::cout << "==============================================" << std::endl;
}

}
