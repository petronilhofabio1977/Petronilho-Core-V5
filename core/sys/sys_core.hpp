// Layer: L0 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// sys_core.hpp - Interface Publica do Subsistema Core
// ================================================================
//
// O QUE ESSE ARQUIVO FAZ:
// Declara a interface publica do modulo sys.
// Qualquer modulo que precise inicializar o Core
// inclui apenas este arquivo, sem expor detalhes internos.
//
// PRINCIPIO: Separacao de interface e implementacao
// BASE TEORICA: Cormen Cap.1 - Design de Algoritmos
// Cormen enfatiza que a interface de um algoritmo deve
// ser separada de sua implementacao. Este header e a
// interface, sys_core.cpp e a implementacao.
// ================================================================

#pragma once

namespace petronilho::sys {

    // Inicializa o subsistema sys, detecta hardware e
    // exibe informacoes de boot. Deve ser chamado uma
    // vez no inicio do programa antes de qualquer
    // alocacao de Arena ou uso de SIMD.
    void boot_info() noexcept;

} // namespace petronilho::sys