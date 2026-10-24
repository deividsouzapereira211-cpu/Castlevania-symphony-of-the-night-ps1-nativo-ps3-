#ifndef MIPS_GEN_H
#define MIPS_GEN_H
#include <stdint.h>
void mips_frame(int px, int py, uint32_t col);
uint32_t mips_reg(int i);
void mips_set_reg(int i, uint32_t v);
uint32_t mips_hi(void);
uint32_t mips_lo(void);
void mips_set_hilo(uint32_t hi, uint32_t lo);
void mips_call(uint32_t pc);
#endif
