#include <string.h>
#include "ps1mem.h"
#include "gp0.h"

#define RAM_SIZE 0x200000

static uint8_t ram[RAM_SIZE];
static uint8_t scratch[1024];
static uint32_t dma_madr[7], dma_bcr[7], dma_chcr[7];
static uint32_t dpcr = 0x07654321, dicr = 0;

static inline uint32_t phys(uint32_t a) { return a & 0x1FFFFFFF; }

static inline uint32_t ld32(const uint8_t *m) {
  return (uint32_t)m[0] | ((uint32_t)m[1] << 8) | ((uint32_t)m[2] << 16) | ((uint32_t)m[3] << 24);
}
static inline void st32(uint8_t *m, uint32_t v) {
  m[0] = (uint8_t)v; m[1] = (uint8_t)(v >> 8); m[2] = (uint8_t)(v >> 16); m[3] = (uint8_t)(v >> 24);
}
static inline uint32_t ram32(uint32_t a) { return ld32(ram + (a & 0x1FFFFC)); }

void ps1_mem_reset(void) {
  memset(ram, 0, sizeof(ram));
  memset(scratch, 0, sizeof(scratch));
  memset(dma_madr, 0, sizeof(dma_madr));
  memset(dma_bcr, 0, sizeof(dma_bcr));
  memset(dma_chcr, 0, sizeof(dma_chcr));
}

static void dma_gpu(int ch) {
  uint32_t chcr = dma_chcr[ch], madr = dma_madr[ch], bcr = dma_bcr[ch];
  int sync = (chcr >> 9) & 3;
  if (sync == 2) {
    uint32_t a = madr & 0x1FFFFC;
    for (int guard = 0; guard < 0x100000; guard++) {
      uint32_t hdr = ram32(a);
      int n = (int)(hdr >> 24);
      for (int i = 0; i < n; i++) gp0_write(ram32(a + 4 + 4 * (uint32_t)i));
      if (hdr & 0x800000) break;
      a = hdr & 0x1FFFFC;
    }
  } else {
    uint32_t n = (sync == 1) ? (bcr & 0xFFFF) * (bcr >> 16) : (bcr & 0xFFFF);
    if (sync == 0 && n == 0) n = 0x10000;
    uint32_t a = madr & 0x1FFFFC;
    for (uint32_t i = 0; i < n; i++, a = (a + 4) & 0x1FFFFC) gp0_write(ram32(a));
  }
}

static void dma_otc(int ch) {
  uint32_t a = dma_madr[ch] & 0x1FFFFC;
  uint32_t n = dma_bcr[ch] & 0xFFFF;
  if (!n) n = 0x10000;
  for (uint32_t i = 0; i < n; i++) {
    uint32_t v = (i == n - 1) ? 0x00FFFFFF : ((a - 4) & 0x1FFFFC);
    st32(ram + a, v);
    a = (a - 4) & 0x1FFFFC;
  }
}

static void dma_write(uint32_t p, uint32_t v) {
  int ch = (int)((p - 0x1F801080) >> 4), reg = (int)((p >> 2) & 3);
  if (ch < 0 || ch > 6) return;
  if (reg == 0) dma_madr[ch] = v & 0xFFFFFF;
  else if (reg == 1) dma_bcr[ch] = v;
  else if (reg == 2) {
    dma_chcr[ch] = v;
    if (v & 0x01000000) {
      if (ch == 2) dma_gpu(ch);
      else if (ch == 6) dma_otc(ch);
      dma_chcr[ch] = v & ~0x11000000u;
    }
  }
}

static uint32_t io_read(uint32_t p) {
  if (p == 0x1F801814) return 0x1C000000;
  if (p >= 0x1F801080 && p < 0x1F8010F0) {
    int ch = (int)((p - 0x1F801080) >> 4), reg = (int)((p >> 2) & 3);
    if (ch <= 6) return reg == 0 ? dma_madr[ch] : reg == 1 ? dma_bcr[ch] : reg == 2 ? dma_chcr[ch] : 0;
  }
  if (p == 0x1F8010F0) return dpcr;
  if (p == 0x1F8010F4) return dicr;
  return 0;
}

static void io_write(uint32_t p, uint32_t v) {
  if (p == 0x1F801810) { gp0_write(v); return; }
  if (p >= 0x1F801080 && p < 0x1F8010F0) { dma_write(p, v); return; }
  if (p == 0x1F8010F0) { dpcr = v; return; }
  if (p == 0x1F8010F4) { dicr = v; return; }
}

uint32_t ps1_r32(uint32_t a) {
  uint32_t p = phys(a);
  if (p < 0x00800000) return ld32(ram + (p & 0x1FFFFC));
  if (p >= 0x1F800000 && p < 0x1F800400) return ld32(scratch + (p & 0x3FC));
  if (p >= 0x1F801000 && p < 0x1F803000) return io_read(p);
  return 0;
}
uint16_t ps1_r16(uint32_t a) {
  uint32_t p = phys(a);
  if (p < 0x00800000) { const uint8_t *m = ram + (p & 0x1FFFFE); return (uint16_t)(m[0] | (m[1] << 8)); }
  if (p >= 0x1F800000 && p < 0x1F800400) { const uint8_t *m = scratch + (p & 0x3FE); return (uint16_t)(m[0] | (m[1] << 8)); }
  if (p >= 0x1F801000 && p < 0x1F803000) return (uint16_t)io_read(p & ~3u);
  return 0;
}
uint8_t ps1_r8(uint32_t a) {
  uint32_t p = phys(a);
  if (p < 0x00800000) return ram[p & 0x1FFFFF];
  if (p >= 0x1F800000 && p < 0x1F800400) return scratch[p & 0x3FF];
  return 0;
}
void ps1_w32(uint32_t a, uint32_t v) {
  uint32_t p = phys(a);
  if (p < 0x00800000) st32(ram + (p & 0x1FFFFC), v);
  else if (p >= 0x1F800000 && p < 0x1F800400) st32(scratch + (p & 0x3FC), v);
  else if (p >= 0x1F801000 && p < 0x1F803000) io_write(p, v);
}
void ps1_w16(uint32_t a, uint16_t v) {
  uint32_t p = phys(a);
  if (p < 0x00800000) { uint8_t *m = ram + (p & 0x1FFFFE); m[0] = (uint8_t)v; m[1] = (uint8_t)(v >> 8); }
  else if (p >= 0x1F800000 && p < 0x1F800400) { uint8_t *m = scratch + (p & 0x3FE); m[0] = (uint8_t)v; m[1] = (uint8_t)(v >> 8); }
}
void ps1_w8(uint32_t a, uint8_t v) {
  uint32_t p = phys(a);
  if (p < 0x00800000) ram[p & 0x1FFFFF] = v;
  else if (p >= 0x1F800000 && p < 0x1F800400) scratch[p & 0x3FF] = v;
}

static uint32_t capbuf[65536];
static int capn;
static void cap_hook(uint32_t w) { if (capn < 65536) capbuf[capn++] = w; }

void ps1_capture_begin(void) { capn = 0; gp0_set_hook(cap_hook); }

void ps1_capture_end(void) {
  gp0_set_hook(0);
  if (capn == 0) return;
  uint32_t a = 0x00100000;
  int i = 0;
  while (i < capn) {
    int n = capn - i;
    if (n > 255) n = 255;
    uint32_t next = (i + n >= capn) ? 0x00FFFFFF : (a + 4 + 4 * (uint32_t)n);
    ps1_w32(a, ((uint32_t)n << 24) | next);
    for (int k = 0; k < n; k++) ps1_w32(a + 4 + 4 * (uint32_t)k, capbuf[i + k]);
    a += 4 + 4 * (uint32_t)n;
    i += n;
  }
  ps1_w32(0x1F8010A0, 0x00100000);
  ps1_w32(0x1F8010A4, 0);
  ps1_w32(0x1F8010A8, 0x01000401);
}


// =================================================================
// SISTEMA HLE v1 - CARREGAMENTO DE ARQUIVOS (DISCO -> RAM NATIVA)
// =================================================================
#include <stdio.h>
#include <string.h>

extern uint8_t ps1_ram_nativa[2 * 1024 * 1024];
extern bool ps1_cpu_regs_interpreter_mode;

void ps1_hle_load_overlay_to_ram(const char* ps1_filename, uint32_t target_vaddr) {
    // No PS3 real, os assets ficam guardados no diretorio de instalacao da USRDIR
    char ps3_path[256];
    snprintf(ps3_path, sizeof(ps3_path), "dev_hdd0/game/PS1RT0001/USRDIR/%s", ps1_filename);
    
    printf("[HLE] Tentando abrir overlay: %s para a RAM: 0x%08X\n", ps1_filename, target_vaddr);
    
    FILE* file = fopen(ps3_path, "rb");
    if (!file) {
        // Fallback local caso esteja testando no Codespaces/PC sem a pasta completa do PS3
        snprintf(ps3_path, sizeof(ps3_path), "share/%s", ps1_filename);
        file = fopen(ps3_path, "rb");
    }

    if (file) {
        // Calcula o offset exato eliminando o prefixo virtual MIPS (0x80000000)
        uint32_t ram_offset = target_vaddr & 0x1FFFFF;
        
        // Le o binario bruto direto para o espaco fisico contiguo da RAM do PS3
        size_t bytes_read = fread(&ps1_ram_nativa[ram_offset], 1, 1.5 * 1024 * 1024, file);
        fclose(file);
        
        printf("[HLE OK] Carregados %zu bytes do arquivo %s na RAM nativa!\n", bytes_read, ps1_filename);
    } else {
        printf("[HLE ERRO] Arquivo de overlay %s nao encontrado no disco!\n", ps1_filename);
    }
}
