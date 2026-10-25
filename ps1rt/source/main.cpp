#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <math.h>
#include <ppu-types.h>

#include <sys/process.h>

#include <io/pad.h>
#include <rsx/rsx.h>
#include <sysutil/sysutil.h>

#include "rsxutil.h"
#include "bitmap.h"
#include "psl1ght.xpm"

u32 running = 0;
u32 frame_cnt = 0;

SYS_PROCESS_PARAM(1001, 0x100000);

extern "C" {
static void program_exit_callback()
{
    sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);
	gcmSetWaitFlip(context);
	rsxFinish(context,1);
}

static void sysutil_exit_callback(u64 status,u64 param,void *usrdata)
{
	switch(status) {
		case SYSUTIL_EXIT_GAME:
			running = 0;
			break;
		case SYSUTIL_DRAW_BEGIN:
		case SYSUTIL_DRAW_END:
			break;
		default:
			break;
	}
}
}

void blit_simple(Bitmap *bitmap, u32 dstX, u32 dstY, u32 srcX, u32 srcY, u32 w, u32 h)
{
  rsxSetTransferImage(context, GCM_TRANSFER_LOCAL_TO_LOCAL,
    color_offset[curr_fb], color_pitch, dstX-w/2, dstY-h/2,
    bitmap->offset, bitmap->width*4, rsxGetFixedUint16((float)srcX),
    rsxGetFixedUint16((float)srcY), w, h, 4);
}

void blit_data(Bitmap *bitmap, u32 dstX, u32 dstY, u32 srcX, u32 srcY, u32 w, u32 h)
{
  rsxSetTransferData(context, GCM_TRANSFER_LOCAL_TO_LOCAL,
    color_offset[curr_fb], color_pitch, bitmap->offset, bitmap->width*4,
    w*4, h);
}

void blit_scale(Bitmap *bitmap, u32 dstX, u32 dstY, u32 srcX, u32 srcY, u32 w, u32 h, float zoom)
{
  gcmTransferScale scale;
  gcmTransferSurface surface;

  scale.conversion = GCM_TRANSFER_CONVERSION_TRUNCATE;
  scale.format = GCM_TRANSFER_SCALE_FORMAT_A8R8G8B8;
  scale.origin = GCM_TRANSFER_ORIGIN_CORNER;
  scale.operation = GCM_TRANSFER_OPERATION_SRCCOPY_AND;
  scale.interp = GCM_TRANSFER_INTERPOLATOR_NEAREST;
  scale.clipX = 0;
  scale.clipY = 0;
  scale.clipW = display_width;
  scale.clipH = display_height;
  scale.outX = dstX - w*zoom*.5f;
  scale.outY = dstY - h*zoom*.5f;
  scale.outW = w * zoom;
  scale.outH = h * zoom;
  scale.ratioX = rsxGetFixedSint32(1.f / zoom);
  scale.ratioY = rsxGetFixedSint32(1.f / zoom);
  scale.inX = rsxGetFixedUint16(srcX);
  scale.inY = rsxGetFixedUint16(srcY);
  scale.inW = bitmap->width;
  scale.inH = bitmap->height;
  scale.offset = bitmap->offset;
  scale.pitch = sizeof(u32) * bitmap->width;

  surface.format = GCM_TRANSFER_SURFACE_FORMAT_A8R8G8B8;
  surface.pitch = color_pitch;
  surface.offset = color_offset[curr_fb];

  rsxSetTransferScaleMode(context, GCM_TRANSFER_LOCAL_TO_LOCAL, GCM_TRANSFER_SURFACE);
  rsxSetTransferScaleSurface(context, &scale, &surface);
}

#include <stdint.h>
#include "gp0.h"
#include "ps1mem.h"
#include "mips_gen.h"

#define SCR_W 320
#define SCR_H 240
enum {
 PS1_SELECT=1<<0, PS1_START=1<<3,
 PS1_UP=1<<4, PS1_RIGHT=1<<5, PS1_DOWN=1<<6, PS1_LEFT=1<<7,
 PS1_L2=1<<8, PS1_R2=1<<9, PS1_L1=1<<10, PS1_R1=1<<11,
 PS1_TRIANGLE=1<<12, PS1_CIRCLE=1<<13, PS1_CROSS=1<<14, PS1_SQUARE=1<<15
};

static inline u32 to_argb(u16 p) {
  u32 r = p & 31, g = (p>>5) & 31, b = (p>>10) & 31;
  r = (r<<3)|(r>>2); g = (g<<3)|(g>>2); b = (b<<3)|(b>>2);
  return 0xFF000000 | (r<<16) | (g<<8) | b;
}

static u16 read_pad2(void) {
  static padData last[MAX_PADS];
  padInfo info; u16 m = 0xFFFF;
  ioPadGetInfo(&info);
  for (int i = 0; i < MAX_PADS; i++) {
    if (!info.status[i]) continue;
    padData d;
    memset(&d, 0, sizeof(d));
    ioPadGetData(i, &d);
    if (d.len > 0) last[i] = d;
    const padData &p = last[i];
#define B(btn, bit) if (p.btn) m &= ~(bit)
    B(BTN_SELECT, PS1_SELECT); B(BTN_START, PS1_START);
    B(BTN_UP, PS1_UP); B(BTN_DOWN, PS1_DOWN);
    B(BTN_LEFT, PS1_LEFT); B(BTN_RIGHT, PS1_RIGHT);
    B(BTN_L1, PS1_L1); B(BTN_R1, PS1_R1);
    B(BTN_L2, PS1_L2); B(BTN_R2, PS1_R2);
    B(BTN_TRIANGLE, PS1_TRIANGLE); B(BTN_CIRCLE, PS1_CIRCLE);
    B(BTN_CROSS, PS1_CROSS); B(BTN_SQUARE, PS1_SQUARE);
#undef B
  }
  return m;
}

int main(int argc, const char *argv[])
{
  void *host_addr = memalign(1024*1024, HOST_SIZE);
  init_screen(host_addr, HOST_SIZE);
  ioPadInit(7);
  atexit(program_exit_callback);
  sysUtilRegisterCallback(0, sysutil_exit_callback, NULL);

  u32 *pix = (u32*)rsxMemalign(64, SCR_W*SCR_H*4);
  u32 off = 0;
  rsxAddressToOffset(pix, &off);
  Bitmap screen;
  screen.width = SCR_W; screen.height = SCR_H; screen.offset = off;

  gp0_reset();
  ps1_mem_reset();
  ps1_capture_begin();
  demo_init();
  ps1_capture_end();
  setRenderTarget(curr_fb);
  int px = 150, py = 110;
  uint32_t col = 0x0000C8FF;
  running = 1;
  while (running) {
    sysUtilCheckCallback();
    u16 pad = read_pad2();
    if (!(pad & PS1_LEFT)) px -= 2;
    if (!(pad & PS1_RIGHT)) px += 2;
    if (!(pad & PS1_UP)) py -= 2;
    if (!(pad & PS1_DOWN)) py += 2;
    if (!(pad & PS1_CROSS)) col = 0x00FF7800;
    if (!(pad & PS1_CIRCLE)) col = 0x002828FF;
    if (!(pad & PS1_SQUARE)) col = 0x00FF3CFF;
    if (!(pad & PS1_TRIANGLE)) col = 0x0050FF28;
    if (!(pad & PS1_SELECT) && !(pad & PS1_START)) running = 0;
    if (px < 0) px = 0;
    if (py < 0) py = 0;
    if (px > SCR_W-16) px = SCR_W-16;
    if (py > SCR_H-16) py = SCR_H-16;

    ps1_capture_begin();
    mips_frame(px, py, col);
    ps1_capture_end();

    for (int y=0; y<SCR_H; y++)
      for (int x=0; x<SCR_W; x++)
        pix[y*SCR_W + x] = to_argb(gpu_vram[y*VRAM_W + x]);

    rsxSetClearColor(context, 0x000000);
    rsxSetClearDepthStencil(context, 0xffff);
    rsxClearSurface(context, GCM_CLEAR_R | GCM_CLEAR_G | GCM_CLEAR_B |
                    GCM_CLEAR_A | GCM_CLEAR_S | GCM_CLEAR_Z);
    rsxSetBlendFunc(context, GCM_SRC_ALPHA, GCM_ONE_MINUS_SRC_ALPHA,
                    GCM_SRC_ALPHA, GCM_ONE_MINUS_SRC_ALPHA);
    rsxSetBlendEquation(context, GCM_FUNC_ADD, GCM_FUNC_ADD);
    rsxSetBlendEnable(context, GCM_TRUE);

    int sc = display_height / SCR_H;
    if (sc < 1) sc = 1;
    if (sc > 3) sc = 3;
    blit_scale(&screen, display_width/2, display_height/2, 0, 0, SCR_W, SCR_H, (float)sc);
    flip();
    frame_cnt++;
  }
  printf("ps1rt finished...\n");
  return 0;
}


// =================================================================
// MOTOR DE EXECUÇÃO HÍBRIDO - TRANSICAO NATIVA / INTERPRETADOR
// =================================================================
extern "C" void execute_recompiled_block(uint32_t pc);
extern "C" void ps1_execute_single_instruction_interpreter();

void ps1_core_hybrid_loop() {
    // Ponto de entrada padrao do SotN apos o boot
    if (ps1_cpu_regs.pc == 0) {
        ps1_cpu_regs.pc = 0x80010DFC;
        ps1_cpu_regs.interpreter_mode = false;
    }

    while (true) {
        if (!ps1_cpu_regs.interpreter_mode) {
            // Roda os blocos traduzidos estaticamente a velocidade maxima no PPE do PS3
            execute_recompiled_block(ps1_cpu_regs.pc);
        } else {
            // Fallback: assume o controle ciclo a ciclo para o DRA.BIN e overlays
            ps1_execute_single_instruction_interpreter();
            
            // Condicao de retorno automatico para o binario nativo estavel
            if (ps1_cpu_regs.pc >= 0x80010000 && ps1_cpu_regs.pc <= 0x8009FFFF) {
                ps1_cpu_regs.interpreter_mode = false;
            }
        }
        
        // Mantem o blit de video ativo na proporcao 4:3 clássica do SotN
        if (ps1_gpu_check_vblank()) {
            ps1_gpu_blit_to_rsx_1080p();
        }
    }
}
