#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "gp0.h"

uint16_t gpu_vram[VRAM_W * VRAM_H];

struct Vtx { int x, y, r, g, b, u, v; };

static struct {
  int tpx, tpy, semi, depth;
  int ax0, ay0, ax1, ay1, ox, oy;
  int mask_set, mask_check;
  int clutx, cluty;
  uint32_t buf[16];
  int n, need;
  int xfer, xx, xy, xw, xh, cx, cy;
} S;

static struct {
  int active, shaded, semi, expect_c, left, have_prev;
  Vtx prev;
  int r, g, b;
} L;

static inline int sx11(uint32_t v) { return (int)((int32_t)(v << 21) >> 21); }
static inline int imin(int a, int b) { return a < b ? a : b; }
static inline int imax(int a, int b) { return a > b ? a : b; }
static inline int clamp31(int v) { return v < 0 ? 0 : (v > 31 ? 31 : v); }

static void set_texpage(uint32_t w) {
  S.tpx = w & 15;
  S.tpy = (w >> 4) & 1;
  S.semi = (w >> 5) & 3;
  S.depth = (w >> 7) & 3;
}

static inline void put(int x, int y, uint16_t c, int blend) {
  if (x < S.ax0 || x > S.ax1 || y < S.ay0 || y > S.ay1) return;
  if ((unsigned)x >= VRAM_W || (unsigned)y >= VRAM_H) return;
  uint16_t *d = &gpu_vram[y * VRAM_W + x];
  if (S.mask_check && (*d & 0x8000)) return;
  if (blend) {
    int br = *d & 31, bg = (*d >> 5) & 31, bb = (*d >> 10) & 31;
    int fr = c & 31, fg = (c >> 5) & 31, fb = (c >> 10) & 31;
    switch (S.semi) {
      case 0: br = (br + fr) >> 1; bg = (bg + fg) >> 1; bb = (bb + fb) >> 1; break;
      case 1: br += fr; bg += fg; bb += fb; break;
      case 2: br -= fr; bg -= fg; bb -= fb; break;
      default: br += fr >> 2; bg += fg >> 2; bb += fb >> 2; break;
    }
    c = (uint16_t)(clamp31(br) | (clamp31(bg) << 5) | (clamp31(bb) << 10) | (c & 0x8000));
  }
  *d = (uint16_t)(c | (S.mask_set ? 0x8000 : 0));
}

static inline uint16_t texel(int u, int v) {
  int bx = S.tpx * 64, by = S.tpy * 256;
  u &= 255; v &= 255;
  if (S.depth == 0) {
    uint16_t w = gpu_vram[((by + v) & 511) * VRAM_W + ((bx + (u >> 2)) & 1023)];
    int idx = (w >> ((u & 3) * 4)) & 15;
    return gpu_vram[(S.cluty & 511) * VRAM_W + ((S.clutx + idx) & 1023)];
  }
  if (S.depth == 1) {
    uint16_t w = gpu_vram[((by + v) & 511) * VRAM_W + ((bx + (u >> 1)) & 1023)];
    int idx = (w >> ((u & 1) * 8)) & 255;
    return gpu_vram[(S.cluty & 511) * VRAM_W + ((S.clutx + idx) & 1023)];
  }
  return gpu_vram[((by + v) & 511) * VRAM_W + ((bx + u) & 1023)];
}

static inline uint16_t modulate(uint16_t t, int cr, int cg, int cb) {
  int r = clamp31((((t & 31) * cr) >> 7));
  int g = clamp31(((((t >> 5) & 31) * cg) >> 7));
  int b = clamp31(((((t >> 10) & 31) * cb) >> 7));
  return (uint16_t)(r | (g << 5) | (b << 10) | (t & 0x8000));
}

static inline void pixel(int x, int y, int r, int g, int b,
                         int tex, int raw, int semi, int u, int v) {
  if (!tex) {
    put(x, y, (uint16_t)((r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)), semi);
    return;
  }
  uint16_t t = texel(u, v);
  if (!t) return;
  uint16_t c = raw ? t : modulate(t, r, g, b);
  put(x, y, c, semi && (t & 0x8000));
}

static void tri(Vtx a, Vtx b, Vtx c, int tex, int raw, int semi) {
  int area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
  if (area == 0) return;
  if (area < 0) { Vtx t = b; b = c; c = t; area = -area; }
  int minx = imin(a.x, imin(b.x, c.x)), maxx = imax(a.x, imax(b.x, c.x));
  int miny = imin(a.y, imin(b.y, c.y)), maxy = imax(a.y, imax(b.y, c.y));
  if (maxx - minx > 1023 || maxy - miny > 511) return;
  minx = imax(minx, S.ax0); maxx = imin(maxx, S.ax1);
  miny = imax(miny, S.ay0); maxy = imin(maxy, S.ay1);
  int dx0 = c.x - b.x, dy0 = c.y - b.y;
  int dx1 = a.x - c.x, dy1 = a.y - c.y;
  int dx2 = b.x - a.x, dy2 = b.y - a.y;
  int tl0 = (dy0 < 0) || (dy0 == 0 && dx0 > 0);
  int tl1 = (dy1 < 0) || (dy1 == 0 && dx1 > 0);
  int tl2 = (dy2 < 0) || (dy2 == 0 && dx2 > 0);
  for (int y = miny; y <= maxy; y++) {
    for (int x = minx; x <= maxx; x++) {
      int w0 = dx0 * (y - b.y) - dy0 * (x - b.x);
      int w1 = dx1 * (y - c.y) - dy1 * (x - c.x);
      int w2 = dx2 * (y - a.y) - dy2 * (x - a.x);
      if (w0 < 0 || w1 < 0 || w2 < 0) continue;
      if ((w0 == 0 && !tl0) || (w1 == 0 && !tl1) || (w2 == 0 && !tl2)) continue;
      int r = (w0 * a.r + w1 * b.r + w2 * c.r) / area;
      int g = (w0 * a.g + w1 * b.g + w2 * c.g) / area;
      int bl = (w0 * a.b + w1 * b.b + w2 * c.b) / area;
      int u = (w0 * a.u + w1 * b.u + w2 * c.u) / area;
      int v = (w0 * a.v + w1 * b.v + w2 * c.v) / area;
      pixel(x, y, r, g, bl, tex, raw, semi, u, v);
    }
  }
}

static void do_poly() {
  const uint32_t *b = S.buf;
  uint32_t op = b[0] >> 24;
  int raw = op & 1, semi = (op >> 1) & 1, tex = (op >> 2) & 1;
  int quad = (op >> 3) & 1, sh = (op >> 4) & 1;
  int nv = quad ? 4 : 3;
  Vtx v[4];
  int i = 1, clut = 0;
  for (int k = 0; k < nv; k++) {
    uint32_t c = (k == 0 || !sh) ? b[0] : b[i++];
    v[k].r = c & 255; v[k].g = (c >> 8) & 255; v[k].b = (c >> 16) & 255;
    uint32_t p = b[i++];
    v[k].x = sx11(p) + S.ox;
    v[k].y = sx11(p >> 16) + S.oy;
    v[k].u = v[k].v = 0;
    if (tex) {
      uint32_t t = b[i++];
      v[k].u = t & 255; v[k].v = (t >> 8) & 255;
      if (k == 0) clut = (int)(t >> 16);
      else if (k == 1) set_texpage(t >> 16);
    }
  }
  S.clutx = (clut & 0x3F) * 16;
  S.cluty = (clut >> 6) & 0x1FF;
  tri(v[0], v[1], v[2], tex, raw, semi);
  if (quad) tri(v[1], v[2], v[3], tex, raw, semi);
}

static void do_rect() {
  const uint32_t *b = S.buf;
  uint32_t op = b[0] >> 24;
  int raw = op & 1, semi = (op >> 1) & 1, tex = (op >> 2) & 1, size = (op >> 3) & 3;
  int r = b[0] & 255, g = (b[0] >> 8) & 255, bl = (b[0] >> 16) & 255;
  int i = 1;
  uint32_t p = b[i++];
  int x0 = sx11(p) + S.ox, y0 = sx11(p >> 16) + S.oy;
  int u0 = 0, v0 = 0;
  if (tex) {
    uint32_t t = b[i++];
    u0 = t & 255; v0 = (t >> 8) & 255;
    S.clutx = ((t >> 16) & 0x3F) * 16;
    S.cluty = (t >> 22) & 0x1FF;
  }
  int w, h;
  if (size == 0) { w = b[i] & 0x3FF; h = (b[i] >> 16) & 0x1FF; }
  else if (size == 1) { w = h = 1; }
  else if (size == 2) { w = h = 8; }
  else { w = h = 16; }
  for (int yy = 0; yy < h; yy++)
    for (int xx = 0; xx < w; xx++)
      pixel(x0 + xx, y0 + yy, r, g, bl, tex, raw, semi, u0 + xx, v0 + yy);
}

static void do_fill() {
  const uint32_t *b = S.buf;
  int x = b[1] & 0x3F0, y = (b[1] >> 16) & 0x1FF;
  int w = (int)(((b[2] & 0x3FF) + 0xF) & ~0xFu), h = (b[2] >> 16) & 0x1FF;
  uint32_t c = b[0];
  uint16_t px = (uint16_t)(((c & 255) >> 3) | ((((c >> 8) & 255) >> 3) << 5) |
                           ((((c >> 16) & 255) >> 3) << 10));
  for (int yy = 0; yy < h; yy++)
    for (int xx = 0; xx < w; xx++)
      gpu_vram[((y + yy) & 511) * VRAM_W + ((x + xx) & 1023)] = px;
}

static void do_vcopy() {
  const uint32_t *b = S.buf;
  int sx = b[1] & 0x3FF, sy = (b[1] >> 16) & 0x1FF;
  int dx = b[2] & 0x3FF, dy = (b[2] >> 16) & 0x1FF;
  int w = (int)(((b[3] & 0x3FF) - 1) & 0x3FF) + 1;
  int h = (int)((((b[3] >> 16) & 0x1FF) - 1) & 0x1FF) + 1;
  for (int yy = 0; yy < h; yy++)
    for (int xx = 0; xx < w; xx++) {
      uint16_t s = gpu_vram[((sy + yy) & 511) * VRAM_W + ((sx + xx) & 1023)];
      uint16_t *d = &gpu_vram[((dy + yy) & 511) * VRAM_W + ((dx + xx) & 1023)];
      if (S.mask_check && (*d & 0x8000)) continue;
      *d = (uint16_t)(s | (S.mask_set ? 0x8000 : 0));
    }
}

static void begin_cpu2vram() {
  const uint32_t *b = S.buf;
  S.xx = b[1] & 0x3FF; S.xy = (b[1] >> 16) & 0x1FF;
  S.xw = (int)(((b[2] & 0x3FF) - 1) & 0x3FF) + 1;
  S.xh = (int)((((b[2] >> 16) & 0x1FF) - 1) & 0x1FF) + 1;
  S.cx = S.cy = 0;
  S.xfer = (S.xw * S.xh + 1) / 2;
}

static void xfer_word(uint32_t w) {
  for (int k = 0; k < 2; k++) {
    if (S.cy < S.xh) {
      uint16_t px = (uint16_t)(w >> (16 * k));
      uint16_t *d = &gpu_vram[((S.xy + S.cy) & 511) * VRAM_W + ((S.xx + S.cx) & 1023)];
      if (!(S.mask_check && (*d & 0x8000))) *d = (uint16_t)(px | (S.mask_set ? 0x8000 : 0));
      if (++S.cx >= S.xw) { S.cx = 0; S.cy++; }
    }
  }
  if (--S.xfer <= 0) S.xfer = 0;
}

static void line_seg(Vtx a, Vtx b, int semi) {
  int dx = abs(b.x - a.x), dy = abs(b.y - a.y);
  if (dx > 1023 || dy > 511) return;
  int n = imax(dx, dy);
  for (int i = 0; i <= n; i++) {
    int x = n ? a.x + (b.x - a.x) * i / n : a.x;
    int y = n ? a.y + (b.y - a.y) * i / n : a.y;
    int r = n ? a.r + (b.r - a.r) * i / n : a.r;
    int g = n ? a.g + (b.g - a.g) * i / n : a.g;
    int bl = n ? a.b + (b.b - a.b) * i / n : a.b;
    pixel(x, y, r, g, bl, 0, 0, semi, 0, 0);
  }
}

static void start_line(uint32_t w) {
  uint32_t op = w >> 24;
  L.active = 1; L.shaded = (op >> 4) & 1; L.semi = (op >> 1) & 1;
  L.left = ((op >> 3) & 1) ? -1 : 2;
  L.expect_c = 0; L.have_prev = 0;
  L.r = w & 255; L.g = (w >> 8) & 255; L.b = (w >> 16) & 255;
}

static void line_word(uint32_t w) {
  if (L.left < 0 && (w & 0xF000F000) == 0x50005000) { L.active = 0; return; }
  if (L.shaded && L.expect_c) {
    L.r = w & 255; L.g = (w >> 8) & 255; L.b = (w >> 16) & 255;
    L.expect_c = 0;
    return;
  }
  Vtx v;
  v.x = sx11(w) + S.ox; v.y = sx11(w >> 16) + S.oy;
  v.r = L.r; v.g = L.g; v.b = L.b; v.u = v.v = 0;
  if (L.have_prev) line_seg(L.prev, v, L.semi);
  L.prev = v; L.have_prev = 1;
  if (L.left > 0 && --L.left == 0) { L.active = 0; return; }
  if (L.shaded) L.expect_c = 1;
}

static int cmd_len(uint32_t w) {
  uint32_t op = w >> 24;
  if (op == 0x02) return 3;
  if (op >= 0x20 && op <= 0x3F) {
    int tex = (op >> 2) & 1, nv = ((op >> 3) & 1) ? 4 : 3, sh = (op >> 4) & 1;
    return 1 + nv * (1 + tex) + (sh ? nv - 1 : 0);
  }
  if (op >= 0x60 && op <= 0x7F) {
    int tex = (op >> 2) & 1, size = (op >> 3) & 3;
    return 2 + tex + (size == 0 ? 1 : 0);
  }
  if (op >= 0x80 && op <= 0x9F) return 4;
  if (op >= 0xA0 && op <= 0xDF) return 3;
  return 1;
}

static void exec_single(uint32_t w) {
  switch (w >> 24) {
    case 0xE1: set_texpage(w); break;
    case 0xE3: S.ax0 = w & 0x3FF; S.ay0 = (w >> 10) & 0x3FF; break;
    case 0xE4: S.ax1 = w & 0x3FF; S.ay1 = (w >> 10) & 0x3FF; break;
    case 0xE5: S.ox = sx11(w); S.oy = sx11(w >> 11); break;
    case 0xE6: S.mask_set = w & 1; S.mask_check = (w >> 1) & 1; break;
    default: break;
  }
}

static void exec_cmd() {
  uint32_t op = S.buf[0] >> 24;
  if (op == 0x02) do_fill();
  else if (op >= 0x20 && op <= 0x3F) do_poly();
  else if (op >= 0x60 && op <= 0x7F) do_rect();
  else if (op >= 0x80 && op <= 0x9F) do_vcopy();
  else if (op >= 0xA0 && op <= 0xBF) begin_cpu2vram();
}

void gp0_reset(void) {
  memset(gpu_vram, 0, sizeof(gpu_vram));
  memset(&S, 0, sizeof(S));
  memset(&L, 0, sizeof(L));
  S.ax1 = 1023; S.ay1 = 511;
}

void gp0_exec(uint32_t w) {
  if (S.xfer) { xfer_word(w); return; }
  if (L.active) { line_word(w); return; }
  if (S.n == 0) {
    uint32_t op = w >> 24;
    if (op >= 0x40 && op <= 0x5F) { start_line(w); return; }
    int len = cmd_len(w);
    if (len == 1) { exec_single(w); return; }
    S.buf[0] = w; S.n = 1; S.need = len;
    return;
  }
  S.buf[S.n++] = w;
  if (S.n >= S.need) {
    S.n = 0;
    exec_cmd();
  }
}

static void (*g_hook)(uint32_t) = 0;
void gp0_set_hook(void (*h)(uint32_t)) { g_hook = h; }
void gp0_write(uint32_t w) { if (g_hook) g_hook(w); else gp0_exec(w); }
