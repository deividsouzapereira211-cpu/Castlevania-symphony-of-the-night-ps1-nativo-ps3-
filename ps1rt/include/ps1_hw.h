#pragma once
#include <cstdint>

// Estrutura global dos registradores da CPU MIPS I do PS1
struct PS1_CPU_Regs {
    uint32_t pc;        // Program Counter
    uint32_t gpr[32];   // Registradores de uso geral
    uint32_t hi;        // Registrador HI para multiplicacao/divisao
    uint32_t lo;        // Registrador LO para multiplicacao/divisao
};

// Tornar os registradores e funcoes visiveis para os arquivos gigantes (.cpp)
extern "C" {
    extern PS1_CPU_Regs ps1_cpu_regs;
    void execute_instruction_fallback(uint32_t pc);
    void trigger_bus_error(uint32_t pc);
}
