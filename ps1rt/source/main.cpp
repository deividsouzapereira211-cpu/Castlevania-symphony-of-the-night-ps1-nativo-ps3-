#include <cstdint>
#include <cstdio>
#include "ps1_hw.h"

// Declaração do roteador central de blocos recompilados do Castlevania
extern "C" void execute_recompiled_block(uint32_t pc);

// Estrutura global dos registradores exigida pelo roteador
PS1_CPU_Regs ps1_cpu_regs;

// Implementações básicas das funções de fallback de barramento
extern "C" {
    void execute_instruction_fallback(uint32_t pc) {
        // Fallback caso saia dos blocos estáticos recompilados
        ps1_cpu_regs.pc = pc + 4;
    }
    void trigger_bus_error(uint32_t pc) {
        // Tratamento de erro de barramento
    }
}

// Ponto de entrada nativo do executável do jogo
int main(int argc, char* argv[]) {
    printf("=============================================\n");
    printf("Inicializando Castlevania: SotN Nativo PS3...\n");
    printf("=============================================\n");

    // Define o PC inicial padrão do executável do PS1 (SLUS_000.67)
    ps1_cpu_regs.pc = 0x80030000; 

    // Loop de execução passando pelo roteador central do jogo
    while (true) {
        execute_recompiled_block(ps1_cpu_regs.pc);
    }

    return 0;
}
