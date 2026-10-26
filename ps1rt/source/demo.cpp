#include <stdint.h>
#include <stdio.h>
#include "ps1mem.h"

// Estrutura global de registradores compartilhada
struct PS1_CPU_State {
    uint32_t gpr[32];
    uint32_t pc;
    uint32_t hi, lo;
    uint32_t ra;
    bool interpreter_mode;
} ps1_cpu_regs;

// Macro para leitura direta na RAM simulada do PS3
#define FETCH_INSTRUCTION(pc) (*(uint32_t*)ps1_translate_address(pc))

extern "C" void ps1_execute_single_instruction_interpreter() {
    uint32_t instr = FETCH_INSTRUCTION(ps1_cpu_regs.pc);
    uint32_t op = instr >> 26;
    uint32_t rs = (instr >> 21) & 0x1F;
    uint32_t rt = (instr >> 16) & 0x1F;
    uint32_t rd = (instr >> 11) & 0x1F;
    int32_t imm = (int16_t)(instr & 0xFFFF);
    uint32_t target = instr & 0x3FFFFFF;
    uint32_t next_pc = ps1_cpu_regs.pc + 4;

    switch (op) {
        case 0x04: // BEQ (Branch if Equal)
            if (ps1_cpu_regs.gpr[rs] == ps1_cpu_regs.gpr[rt]) {
                next_pc = ps1_cpu_regs.pc + 4 + (imm << 2);
            }
            break;
            
        case 0x05: // BNE (Branch if Not Equal)
            if (ps1_cpu_regs.gpr[rs] != ps1_cpu_regs.gpr[rt]) {
                next_pc = ps1_cpu_regs.pc + 4 + (imm << 2);
            }
            break;

        case 0x06: // BLEZ (Branch if Less Than or Equal to Zero)
            if ((int32_t)ps1_cpu_regs.gpr[rs] <= 0) {
                next_pc = ps1_cpu_regs.pc + 4 + (imm << 2);
            }
            break;

        case 0x07: // BGTZ (Branch if Greater Than Zero)
            if ((int32_t)ps1_cpu_regs.gpr[rs] > 0) {
                next_pc = ps1_cpu_regs.pc + 4 + (imm << 2);
            }
            break;

        case 0x00: // SPECIAL (Registradores R-Type)
            if ((instr & 0x3F) == 0x21) { // ADDU
                ps1_cpu_regs.gpr[rd] = ps1_cpu_regs.gpr[rs] + ps1_cpu_regs.gpr[rt];
            }
            if ((instr & 0x3F) == 0x08) { // JR
                next_pc = ps1_cpu_regs.gpr[rs];
            }
            break;
        case 0x09: // ADDIU
            ps1_cpu_regs.gpr[rt] = ps1_cpu_regs.gpr[rs] + imm;
            break;
        case 0x23: // LW
            ps1_cpu_regs.gpr[rt] = *(uint32_t*)ps1_translate_address(ps1_cpu_regs.gpr[rs] + imm);
            break;
        case 0x2B: // SW
            *(uint32_t*)ps1_translate_address(ps1_cpu_regs.gpr[rs] + imm) = ps1_cpu_regs.gpr[rt];
            break;
        case 0x02: // J
            next_pc = (ps1_cpu_regs.pc & 0xF0000000) | (target << 2);
            break;
    }
    ps1_cpu_regs.pc = next_pc;
}
