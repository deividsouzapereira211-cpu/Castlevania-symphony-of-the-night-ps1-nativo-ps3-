#ifndef PS1MEM_H
#define PS1MEM_H
#include <stdint.h>

void ps1_mem_reset(void);
uint8_t  ps1_r8(uint32_t a);
uint16_t ps1_r16(uint32_t a);
uint32_t ps1_r32(uint32_t a);
void ps1_w8(uint32_t a, uint8_t v);
void ps1_w16(uint32_t a, uint16_t v);
void ps1_w32(uint32_t a, uint32_t v);

void ps1_capture_begin(void);
void ps1_capture_end(void);

#endif
