#include <cstdint>

// Declarações externas explícitas em C combinando com os arquivos gerados
extern "C" {
    void execute_recompiled_block_main(uint32_t pc);
    void execute_recompiled_block_dra(uint32_t pc);
    void execute_recompiled_block(uint32_t pc);
}

void execute_recompiled_block(uint32_t pc) {
    if (pc >= 0x800A0000) {
        execute_recompiled_block_dra(pc);
    } else {
        execute_recompiled_block_main(pc);
    }
}
