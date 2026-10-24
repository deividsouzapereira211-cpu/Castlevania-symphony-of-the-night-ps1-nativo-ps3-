// gerado por mipsrecomp.py - NAO EDITE
#include <stdint.h>
#include <stdio.h>
#include "ps1mem.h"
#include "mips_gen.h"

static uint32_t R[32], C0[32], HI, LO;
#define RET_ADDR 0xFFFF0000u

static void run(uint32_t pc) {
  uint32_t c, tgt;
  int64_t m64;
  uint64_t u64;
  (void)c; (void)tgt; (void)m64; (void)u64;
dispatch:
  switch (pc) {
  case 0x80010000u: goto L_80010000;
  case 0x8001006Cu: goto L_8001006C;
  case 0x800100A4u: goto L_800100A4;
  case 0x800100D0u: goto L_800100D0;
  case 0x800100E0u: goto L_800100E0;
  case 0x80010104u: goto L_80010104;
  case 0x80010118u: goto L_80010118;
  case 0x80010138u: goto L_80010138;
  case 0x8001015Cu: goto L_8001015C;
  case 0x8001016Cu: goto L_8001016C;
  case 0x80010190u: goto L_80010190;
  case 0x800101ACu: goto L_800101AC;
  case 0x800101C0u: goto L_800101C0;
  case 0x800101D8u: goto L_800101D8;
  case 0x80010200u: goto L_80010200;
  case 0x80010230u: goto L_80010230;
  case 0x80010244u: goto L_80010244;
  case 0x80010248u: goto L_80010248;
  case 0x80010250u: goto L_80010250;
  case 0x80010254u: goto L_80010254;
  case 0x8001025Cu: goto L_8001025C;
  case 0x80010260u: goto L_80010260;
  case 0x80010268u: goto L_80010268;
  case 0x8001026Cu: goto L_8001026C;
  case 0x8001027Cu: goto L_8001027C;
  case 0x8001028Cu: goto L_8001028C;
  case 0x80010290u: goto L_80010290;
  case 0x800102A4u: goto L_800102A4;
  case 0x800102B4u: goto L_800102B4;
  case 0x800102C4u: goto L_800102C4;
  case 0x800102E4u: goto L_800102E4;
  case 0x80010318u: goto L_80010318;
  case 0x8001032Cu: goto L_8001032C;
  case 0x80010344u: goto L_80010344;
  case 0x80010364u: goto L_80010364;
  case 0x80010370u: goto L_80010370;
  case 0x800103CCu: goto L_800103CC;
  case RET_ADDR: return;
  default: printf("mips: salto para %08X sem codigo\n", (unsigned)pc); return;
  }
L_80010000:
  R[29] = R[29] + 4294967264u;
  ps1_w32((R[29] + 28u), R[31]);
  ps1_w32((R[29] + 24u), R[16]);
  ps1_w32((R[29] + 20u), R[17]);
  ps1_w32((R[29] + 16u), R[18]);
  ps1_w32((R[29] + 12u), R[19]);
  R[17] = R[0] + R[4];
  R[18] = R[0] + R[5];
  R[19] = R[0] + R[6];
  R[16] = 2148532224u;
  R[8] = 528482304u;
  R[8] = R[8] | 6160u;
  R[9] = 3808428032u;
  ps1_w32(R[8], R[9]);
  R[9] = 3825401856u;
  R[9] = R[9] | 48447u;
  ps1_w32(R[8], R[9]);
  R[9] = 3841982464u;
  ps1_w32(R[8], R[9]);
  R[4] = R[0] + R[0];
  R[5] = R[0] + R[0];
  R[6] = 15728640u;
  R[6] = R[6] | 320u;
  R[7] = 4194304u;
  R[7] = R[7] | 8208u;
  {
    R[31] = 0x8001006Cu;
    goto L_800103CC; }
L_8001006C:
  R[8] = 1144193024u;
  R[8] = R[8] | 8721u;
  ps1_w32(R[16], R[8]);
  R[8] = 2289500160u;
  R[8] = R[8] | 26197u;
  ps1_w32((R[16] + 4u), R[8]);
  R[2] = R[0] + R[0];
  { uint32_t a = (R[16] + 1u), v = ps1_r32(a & ~3u), r = R[2]; switch (a & 3) { case 0: r = v; break; case 1: r = (r & 0xFF000000u) | (v >> 8); break; case 2: r = (r & 0xFFFF0000u) | (v >> 16); break; default: r = (r & 0xFFFFFF00u) | (v >> 24); break; } R[2] = r; }
  { uint32_t a = (R[16] + 4u), v = ps1_r32(a & ~3u), r = R[2]; switch (a & 3) { case 0: r = (r & 0x00FFFFFFu) | (v << 24); break; case 1: r = (r & 0x0000FFFFu) | (v << 16); break; case 2: r = (r & 0x000000FFu) | (v << 8); break; default: r = v; break; } R[2] = r; }
  R[5] = R[0] + R[2];
  R[6] = 1430519808u;
  R[6] = R[6] | 13090u;
  {
    R[31] = 0x800100A4u;
    R[4] = R[0] + 0u;
    goto L_80010370; }
L_800100A4:
  ps1_w32((R[16] + 8u), R[0]);
  ps1_w32((R[16] + 12u), R[0]);
  R[8] = 2864381952u;
  R[8] = R[8] | 52445u;
  { uint32_t a = (R[16] + 9u), v = ps1_r32(a & ~3u), r = R[8]; switch (a & 3) { case 0: v = r; break; case 1: v = (v & 0x000000FFu) | (r << 8); break; case 2: v = (v & 0x0000FFFFu) | (r << 16); break; default: v = (v & 0x00FFFFFFu) | (r << 24); break; } ps1_w32(a & ~3u, v); }
  { uint32_t a = (R[16] + 12u), v = ps1_r32(a & ~3u), r = R[8]; switch (a & 3) { case 0: v = (v & 0xFFFFFF00u) | (r >> 24); break; case 1: v = (v & 0xFFFF0000u) | (r >> 16); break; case 2: v = (v & 0xFF000000u) | (r >> 8); break; default: v = r; break; } ps1_w32(a & ~3u, v); }
  R[5] = ps1_r32((R[16] + 8u));
  R[6] = 3150708736u;
  R[6] = R[6] | 56576u;
  {
    R[31] = 0x800100D0u;
    R[4] = R[0] + 1u;
    goto L_80010370; }
L_800100D0:
  R[5] = ps1_r32((R[16] + 12u));
  R[6] = R[0] | 170u;
  {
    R[31] = 0x800100E0u;
    R[4] = R[0] + 2u;
    goto L_80010370; }
L_800100E0:
  R[8] = 4294901760u;
  R[8] = R[8] | 65533u;
  R[9] = R[0] | 5u;
  m64 = (int64_t)(int32_t)R[8] * (int64_t)(int32_t)R[9]; LO = (uint32_t)m64; HI = (uint32_t)(m64 >> 32);
  R[5] = LO;
  R[6] = 4294901760u;
  R[6] = R[6] | 65521u;
  {
    R[31] = 0x80010104u;
    R[4] = R[0] + 3u;
    goto L_80010370; }
L_80010104:
  R[5] = HI;
  R[6] = 4294901760u;
  R[6] = R[6] | 65535u;
  {
    R[31] = 0x80010118u;
    R[4] = R[0] + 4u;
    goto L_80010370; }
L_80010118:
  R[8] = 4294901760u;
  R[8] = R[8] | 65535u;
  R[9] = R[0] | 2u;
  u64 = (uint64_t)R[8] * (uint64_t)R[9]; LO = (uint32_t)u64; HI = (uint32_t)(u64 >> 32);
  R[5] = HI;
  R[6] = R[0] | 1u;
  {
    R[31] = 0x80010138u;
    R[4] = R[0] + 5u;
    goto L_80010370; }
L_80010138:
  R[8] = R[0] | 7u;
  R[9] = 4294901760u;
  R[9] = R[9] | 65534u;
  { int32_t n = (int32_t)R[8], d = (int32_t)R[9]; if (d == 0) { LO = (n >= 0) ? 0xFFFFFFFFu : 1u; HI = (uint32_t)n; } else if ((uint32_t)n == 0x80000000u && d == -1) { LO = 0x80000000u; HI = 0; } else { LO = (uint32_t)(n / d); HI = (uint32_t)(n % d); } }
  R[5] = LO;
  R[6] = 4294901760u;
  R[6] = R[6] | 65533u;
  {
    R[31] = 0x8001015Cu;
    R[4] = R[0] + 6u;
    goto L_80010370; }
L_8001015C:
  R[5] = HI;
  R[6] = R[0] | 1u;
  {
    R[31] = 0x8001016Cu;
    R[4] = R[0] + 7u;
    goto L_80010370; }
L_8001016C:
  R[8] = 4294901760u;
  R[8] = R[8] | 65534u;
  R[9] = R[0] | 3u;
  { uint32_t n = R[8], d = R[9]; if (d == 0) { LO = 0xFFFFFFFFu; HI = n; } else { LO = n / d; HI = n % d; } }
  R[5] = LO;
  R[6] = 1431633920u;
  R[6] = R[6] | 21844u;
  {
    R[31] = 0x80010190u;
    R[4] = R[0] + 8u;
    goto L_80010370; }
L_80010190:
  R[8] = R[0] | 5u;
  { int32_t n = (int32_t)R[8], d = (int32_t)R[0]; if (d == 0) { LO = (n >= 0) ? 0xFFFFFFFFu : 1u; HI = (uint32_t)n; } else if ((uint32_t)n == 0x80000000u && d == -1) { LO = 0x80000000u; HI = 0; } else { LO = (uint32_t)(n / d); HI = (uint32_t)(n % d); } }
  R[5] = LO;
  R[6] = 4294901760u;
  R[6] = R[6] | 65535u;
  {
    R[31] = 0x800101ACu;
    R[4] = R[0] + 9u;
    goto L_80010370; }
L_800101AC:
  R[8] = 2147483648u;
  R[5] = (uint32_t)((int32_t)R[8] >> 4);
  R[6] = 4160749568u;
  {
    R[31] = 0x800101C0u;
    R[4] = R[0] + 10u;
    goto L_80010370; }
L_800101C0:
  R[8] = 2147483648u;
  R[9] = R[0] | 36u;
  R[5] = (uint32_t)((int32_t)R[8] >> (R[9] & 31));
  R[6] = 4160749568u;
  {
    R[31] = 0x800101D8u;
    R[4] = R[0] + 11u;
    goto L_80010370; }
L_800101D8:
  R[8] = 4294901760u;
  R[8] = R[8] | 65535u;
  R[9] = R[0] | 1u;
  R[10] = (uint32_t)((int32_t)R[8] < (int32_t)R[9]);
  R[11] = (uint32_t)(R[8] < R[9]);
  R[10] = R[10] << 1;
  R[5] = R[10] | R[11];
  R[6] = R[0] | 2u;
  {
    R[31] = 0x80010200u;
    R[4] = R[0] + 12u;
    goto L_80010370; }
L_80010200:
  R[8] = R[0] | 32896u;
  ps1_w32((R[16] + 16u), R[8]);
  R[9] = (uint32_t)(int32_t)(int8_t)ps1_r8((R[16] + 16u));
  R[10] = ps1_r8((R[16] + 16u));
  R[11] = (uint32_t)(int32_t)(int16_t)ps1_r16((R[16] + 16u));
  R[12] = ps1_r16((R[16] + 16u));
  R[5] = R[9] ^ R[10];
  R[5] = R[5] ^ R[11];
  R[5] = R[5] ^ R[12];
  R[6] = R[0] | 65280u;
  {
    R[31] = 0x80010230u;
    R[4] = R[0] + 13u;
    goto L_80010370; }
L_80010230:
  R[2] = R[0] + R[0];
  R[8] = 4294901760u;
  R[8] = R[8] | 65535u;
  { c = (uint32_t)((int32_t)R[8] < 0);
    if (c) goto L_80010248; }
L_80010244:
  R[2] = R[2] | 1u;
L_80010248:
  { c = (uint32_t)((int32_t)R[8] >= 0);
    if (c) goto L_80010254; }
L_80010250:
  R[2] = R[2] | 2u;
L_80010254:
  { c = (uint32_t)((int32_t)R[8] <= 0);
    if (c) goto L_80010260; }
L_8001025C:
  R[2] = R[2] | 4u;
L_80010260:
  { c = (uint32_t)((int32_t)R[8] > 0);
    if (c) goto L_8001026C; }
L_80010268:
  R[2] = R[2] | 8u;
L_8001026C:
  R[5] = R[0] + R[2];
  R[6] = R[0] | 10u;
  {
    R[31] = 0x8001027Cu;
    R[4] = R[0] + 14u;
    goto L_80010370; }
L_8001027C:
  R[8] = R[0] | 1u;
  R[2] = R[0] + R[0];
  { c = (uint32_t)(R[8] != R[0]);
    R[8] = R[0] + R[0];
    if (c) goto L_80010290; }
L_8001028C:
  R[2] = R[2] | 1u;
L_80010290:
  R[2] = R[2] | 2u;
  R[5] = R[2] + R[8];
  R[6] = R[0] | 2u;
  {
    R[31] = 0x800102A4u;
    R[4] = R[0] + 15u;
    goto L_80010370; }
L_800102A4:
  R[8] = 2147549184u;
  R[8] = R[8] | 868u;
  { tgt = R[8];
    R[31] = 0x800102B4u;
    R[2] = R[0] + 7u;
    pc = tgt; goto dispatch; }
L_800102B4:
  R[5] = R[0] + R[2];
  R[6] = R[0] | 8u;
  {
    R[31] = 0x800102C4u;
    R[4] = R[0] + 16u;
    goto L_80010370; }
L_800102C4:
  R[8] = 305397760u;
  R[8] = R[8] | 22136u;
  C0[13] = R[8];
  R[5] = C0[13];
  R[6] = 305397760u;
  R[6] = R[6] | 22136u;
  {
    R[31] = 0x800102E4u;
    R[4] = R[0] + 17u;
    goto L_80010370; }
L_800102E4:
  R[8] = R[0] + 1u;
  ps1_w8((R[16] + 20u), (uint8_t)R[8]);
  R[8] = R[0] + 2u;
  ps1_w8((R[16] + 21u), (uint8_t)R[8]);
  R[8] = R[0] + 3u;
  ps1_w8((R[16] + 22u), (uint8_t)R[8]);
  R[8] = R[0] + 4u;
  ps1_w8((R[16] + 23u), (uint8_t)R[8]);
  R[5] = ps1_r32((R[16] + 20u));
  R[6] = 67305472u;
  R[6] = R[6] | 513u;
  {
    R[31] = 0x80010318u;
    R[4] = R[0] + 18u;
    goto L_80010370; }
L_80010318:
  R[5] = ~(R[0] | R[0]);
  R[6] = 4294901760u;
  R[6] = R[6] | 65535u;
  {
    R[31] = 0x8001032Cu;
    R[4] = R[0] + 19u;
    goto L_80010370; }
L_8001032C:
  R[4] = R[0] + R[17];
  R[5] = R[0] + R[18];
  R[6] = 1048576u;
  R[6] = R[6] | 16u;
  {
    R[31] = 0x80010344u;
    R[7] = R[0] + R[19];
    goto L_800103CC; }
L_80010344:
  R[19] = ps1_r32((R[29] + 12u));
  R[18] = ps1_r32((R[29] + 16u));
  R[17] = ps1_r32((R[29] + 20u));
  R[16] = ps1_r32((R[29] + 24u));
  R[31] = ps1_r32((R[29] + 28u));
  R[29] = R[29] + 32u;
  { tgt = R[31];
    pc = tgt; goto dispatch; }
L_80010364:
  R[2] = R[2] + 1u;
  { tgt = R[31];
    pc = tgt; goto dispatch; }
L_80010370:
  R[24] = R[5] ^ R[6];
  R[24] = (uint32_t)(R[24] < 1u);
  R[25] = R[0] - R[24];
  R[15] = R[25] & 65280u;
  R[25] = ~(R[25] | R[0]);
  R[25] = R[25] & 255u;
  R[7] = R[15] | R[25];
  R[8] = R[4] & 15u;
  R[9] = R[8] << 4;
  R[10] = R[8] << 1;
  R[9] = R[9] + R[10];
  R[11] = R[9] + 8u;
  R[12] = R[4] >> 4;
  R[13] = R[12] << 4;
  R[14] = R[12] << 3;
  R[13] = R[13] + R[14];
  R[13] = R[13] + 8u;
  R[4] = R[0] + R[11];
  R[5] = R[0] + R[13];
  R[6] = 1048576u;
  R[6] = R[6] | 16u;
  {
    goto L_800103CC; }
L_800103CC:
  R[8] = 528482304u;
  R[8] = R[8] | 6160u;
  R[9] = 1610612736u;
  R[9] = R[9] | R[7];
  ps1_w32(R[8], R[9]);
  R[10] = R[5] << 16;
  R[11] = R[4] & 2047u;
  R[10] = R[10] | R[11];
  ps1_w32(R[8], R[10]);
  ps1_w32(R[8], R[6]);
  { tgt = R[31];
    pc = tgt; goto dispatch; }
  goto dispatch;
}

uint32_t mips_reg(int i) { return R[i & 31]; }
void mips_set_reg(int i, uint32_t v) { if (i & 31) R[i & 31] = v; }
uint32_t mips_hi(void) { return HI; }
uint32_t mips_lo(void) { return LO; }
void mips_set_hilo(uint32_t hi, uint32_t lo) { HI = hi; LO = lo; }
void mips_call(uint32_t pc) { R[31] = RET_ADDR; run(pc); }

void mips_frame(int px, int py, uint32_t col) {
  R[4] = (uint32_t)px; R[5] = (uint32_t)py; R[6] = col;
  R[29] = 0x801FFF00u; R[31] = RET_ADDR;
  run(0x80010000u);
}
