// gerado por mipsrecomp.py - NAO EDITE
#include <stdint.h>
#include <stdio.h>
#include "ps1mem.h"
#include "mips_gen.h"

static uint32_t R[32];
static uint32_t HI __attribute__((unused)), LO __attribute__((unused));
#define RET_ADDR 0xFFFF0000u

static void run(uint32_t pc) {
  uint32_t c, tgt;
  int64_t m64;
  uint64_t u64;
  (void)c; (void)tgt; (void)m64; (void)u64;
dispatch:
  switch (pc) {
  case 0x80010000u: goto L_80010000;
  case 0x8001004Cu: goto L_8001004C;
  case 0x80010084u: goto L_80010084;
  case 0x80010094u: goto L_80010094;
  case 0x800100ACu: goto L_800100AC;
  case 0x800100CCu: goto L_800100CC;
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
  R[16] = R[0] + R[4];
  R[17] = R[0] + R[5];
  R[18] = R[0] + R[6];
  R[8] = 528482304u;
  R[8] = R[8] | 6160u;
  R[9] = 3808428032u;
  ps1_w32(R[8], R[9]);
  R[9] = 3825401856u;
  R[9] = R[9] | 48447u;
  ps1_w32(R[8], R[9]);
  R[9] = 3841982464u;
  ps1_w32(R[8], R[9]);
  R[19] = R[0] + R[0];
L_8001004C:
  R[10] = R[19] << 5;
  R[11] = R[19] << 3;
  R[4] = R[10] + R[11];
  R[5] = R[0] + R[0];
  R[6] = 15728640u;
  R[6] = R[6] | 40u;
  R[13] = R[0] | 255u;
  R[13] = R[13] - R[10];
  R[13] = R[13] << 8;
  R[7] = R[10] | R[13];
  R[14] = 8388608u;
  R[7] = R[7] | R[14];
  {
    R[31] = 0x80010084u;
    goto L_800100CC; }
L_80010084:
  R[19] = R[19] + 1u;
  R[15] = (uint32_t)((int32_t)R[19] < 8);
  { c = (uint32_t)(R[15] != R[0]);
    if (c) goto L_8001004C; }
L_80010094:
  R[4] = R[0] + R[16];
  R[5] = R[0] + R[17];
  R[6] = 1048576u;
  R[6] = R[6] | 16u;
  {
    R[31] = 0x800100ACu;
    R[7] = R[0] + R[18];
    goto L_800100CC; }
L_800100AC:
  R[19] = ps1_r32((R[29] + 12u));
  R[18] = ps1_r32((R[29] + 16u));
  R[17] = ps1_r32((R[29] + 20u));
  R[16] = ps1_r32((R[29] + 24u));
  R[31] = ps1_r32((R[29] + 28u));
  R[29] = R[29] + 32u;
  { tgt = R[31];
    pc = tgt; goto dispatch; }
L_800100CC:
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

void mips_frame(int px, int py, uint32_t col) {
  R[4] = (uint32_t)px; R[5] = (uint32_t)py; R[6] = col;
  R[29] = 0x801FFF00u; R[31] = RET_ADDR;
  run(0x80010000u);
}
