#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "gp0.h"

static inline uint32_t xy(int x, int y) {
  return ((uint32_t)(y & 0x7FF) << 16) | (uint32_t)(x & 0x7FF);
}
static inline uint32_t rgb(int r, int g, int b) {
  return ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
}
static inline uint16_t c555(int r, int g, int b) {
  return (uint16_t)((r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10));
}

void demo_init(void) {
  gp0_write(0xE3000000);
  gp0_write(0xE4000000 | 319 | (239 << 10));
  gp0_write(0xE5000000);

  gp0_write(0xA0000000); gp0_write(xy(640, 0)); gp0_write(xy(4, 16));
  for (int y = 0; y < 16; y++) {
    uint16_t row[4] = {0, 0, 0, 0};
    for (int x = 0; x < 16; x++) {
      int d = abs(2 * x - 15) + abs(2 * y - 15);
      int idx = (d <= 14) ? (d >= 12 ? 2 : 1) : 0;
      row[x >> 2] |= (uint16_t)(idx << ((x & 3) * 4));
    }
    gp0_write(row[0] | ((uint32_t)row[1] << 16));
    gp0_write(row[2] | ((uint32_t)row[3] << 16));
  }

  gp0_write(0xA0000000); gp0_write(xy(640, 256)); gp0_write(xy(16, 1));
  uint16_t clut[16] = {0};
  clut[1] = c555(255, 160, 0);
  clut[2] = c555(255, 255, 255);
  for (int i = 0; i < 16; i += 2)
    gp0_write(clut[i] | ((uint32_t)clut[i + 1] << 16));
}

void demo_frame(int px, int py, uint32_t col, int frame) {
  const uint32_t TP = 0xE100040A;
  gp0_write(0xE1000400);
  gp0_write(0xE6000000);

  gp0_write(0x38000000 | rgb(20, 20, 140)); gp0_write(xy(0, 0));
  gp0_write(rgb(20, 20, 140));              gp0_write(xy(320, 0));
  gp0_write(rgb(20, 230, 120));             gp0_write(xy(0, 240));
  gp0_write(rgb(20, 230, 120));             gp0_write(xy(320, 240));

  int tx = (int)(20.0 * sin(frame * 0.05));
  gp0_write(0xE1000400);
  gp0_write(0x32000000 | rgb(255, 0, 0));   gp0_write(xy(60 + tx, 120));
  gp0_write(rgb(0, 255, 0));                gp0_write(xy(20 + tx, 200));
  gp0_write(rgb(0, 0, 255));                gp0_write(xy(110 + tx, 200));

  gp0_write(0xE1000420);
  gp0_write(0x62000000 | rgb(50, 50, 50));
  gp0_write(xy(0, 210)); gp0_write(xy(320, 30));
  gp0_write(0xE1000400);

  gp0_write(0x60000000 | col);
  gp0_write(xy(px, py)); gp0_write(xy(16, 16));

  gp0_write(TP);
  gp0_write(0x64808080);
  gp0_write(xy(px + 24, py));
  gp0_write(0x40280000);
  gp0_write(xy(16, 16));

  gp0_write(0x2C808080);
  gp0_write(xy(240, 30));  gp0_write(0x40280000);
  gp0_write(xy(304, 30));  gp0_write(0x000A000F);
  gp0_write(xy(240, 94));  gp0_write(0x00000F00);
  gp0_write(xy(304, 94));  gp0_write(0x00000F0F);

  gp0_write(0x80000000);
  gp0_write(xy(px + 24, py)); gp0_write(xy(8, 8)); gp0_write(xy(16, 16));
}
