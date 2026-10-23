#ifndef GP0_H
#define GP0_H
#include <stdint.h>

#define VRAM_W 1024
#define VRAM_H 512

extern uint16_t gpu_vram[VRAM_W * VRAM_H];

void gp0_reset(void);
void gp0_write(uint32_t w);

void demo_init(void);
void demo_frame(int px, int py, uint32_t rgb, int frame);

void gp0_exec(uint32_t w);
void gp0_set_hook(void (*h)(uint32_t));
#endif
