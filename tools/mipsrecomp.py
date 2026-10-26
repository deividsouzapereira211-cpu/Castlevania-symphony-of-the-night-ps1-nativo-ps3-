#!/usr/bin/env python3
# mipsrecomp.py - recompilador estatico MIPS R3000 (PS1) -> C++  (v2)
# Inclui um mini assembler. Le tools/selftest.asm (autoteste que desenha pelo GPU).
# Uso: python3 tools/mipsrecomp.py ps1rt
import sys, os

REGS = ["zero","at","v0","v1","a0","a1","a2","a3","t0","t1","t2","t3","t4","t5","t6","t7",
        "s0","s1","s2","s3","s4","s5","s6","s7","t8","t9","k0","k1","gp","sp","fp","ra"]
RN = {n: i for i, n in enumerate(REGS)}

def reg(s):
    s = s.strip().lstrip("$")
    return RN[s] if s in RN else int(s)

# ------------------------------------------------------------------ assembler
RTYPE = {"addu":0x21,"subu":0x23,"and":0x24,"or":0x25,"xor":0x26,"nor":0x27,"slt":0x2A,"sltu":0x2B}
ITYPE = {"addiu":9,"slti":10,"sltiu":11,"andi":12,"ori":13,"xori":14}
MEM = {"lb":0x20,"lh":0x21,"lwl":0x22,"lw":0x23,"lbu":0x24,"lhu":0x25,"lwr":0x26,
       "sb":0x28,"sh":0x29,"swl":0x2A,"sw":0x2B,"swr":0x2E}
SHV = {"sllv":4,"srlv":6,"srav":7}
MULDIV = {"mult":24,"multu":25,"div":26,"divu":27}

def assemble(src, base):
    items, labels = [], {}
    for line in src.splitlines():
        line = line.split("#")[0].strip()
        while ":" in line:
            lab, line = line.split(":", 1)
            labels[lab.strip()] = base + 4 * len(items)
            line = line.strip()
        if not line:
            continue
        m, _, rest = line.partition(" ")
        a = [x.strip() for x in rest.split(",")] if rest.strip() else []
        if m == "nop":
            items.append(("sll", ["zero", "zero", "0"]))
        elif m == "move":
            items.append(("addu", [a[0], "zero", a[1]]))
        elif m == "la":
            items.append(("lui", [a[0], "@hi:" + a[1]]))
            items.append(("ori", [a[0], a[0], "@lo:" + a[1]]))
        elif m == "li":
            v = int(a[1], 0) & 0xFFFFFFFF
            if (v >> 16) == 0:
                items.append(("ori", [a[0], "zero", str(v)]))
            elif (v & 0xFFFF) == 0:
                items.append(("lui", [a[0], str(v >> 16)]))
            else:
                items.append(("lui", [a[0], str(v >> 16)]))
                items.append(("ori", [a[0], a[0], str(v & 0xFFFF)]))
        else:
            items.append((m, a))
    def num(x):
        if x.startswith("@hi:"): return labels[x[4:]] >> 16
        if x.startswith("@lo:"): return labels[x[4:]] & 0xFFFF
        return int(x, 0)
    words = []
    for i, (m, a) in enumerate(items):
        pc = base + 4 * i
        if m in RTYPE:
            w = (reg(a[1]) << 21) | (reg(a[2]) << 16) | (reg(a[0]) << 11) | RTYPE[m]
        elif m in ("sll", "srl", "sra"):
            fn = {"sll": 0, "srl": 2, "sra": 3}[m]
            w = (reg(a[1]) << 16) | (reg(a[0]) << 11) | (int(a[2], 0) << 6) | fn
        elif m in SHV:
            w = (reg(a[2]) << 21) | (reg(a[1]) << 16) | (reg(a[0]) << 11) | SHV[m]
        elif m in MULDIV:
            w = (reg(a[0]) << 21) | (reg(a[1]) << 16) | MULDIV[m]
        elif m in ("mfhi", "mflo"):
            w = (reg(a[0]) << 11) | (16 if m == "mfhi" else 18)
        elif m in ("mthi", "mtlo"):
            w = (reg(a[0]) << 21) | (17 if m == "mthi" else 19)
        elif m in ("mfc0", "mtc0"):
            w = (16 << 26) | ((0 if m == "mfc0" else 4) << 21) | (reg(a[0]) << 16) | (reg(a[1]) << 11)
        elif m in ("bltz", "bgez", "blez", "bgtz"):
            off = (labels[a[1]] - (pc + 4)) >> 2
            if m in ("bltz", "bgez"):
                w = (1 << 26) | (reg(a[0]) << 21) | ((0 if m == "bltz" else 1) << 16) | (off & 0xFFFF)
            else:
                w = ((6 if m == "blez" else 7) << 26) | (reg(a[0]) << 21) | (off & 0xFFFF)
        elif m == "jalr":
            w = (reg(a[0]) << 21) | (31 << 11) | 9
        elif m in ITYPE:
            w = (ITYPE[m] << 26) | (reg(a[1]) << 21) | (reg(a[0]) << 16) | (num(a[2]) & 0xFFFF)
        elif m == "lui":
            w = (15 << 26) | (reg(a[0]) << 16) | (num(a[1]) & 0xFFFF)
        elif m in MEM:
            off, _, r = a[1].partition("(")
            w = (MEM[m] << 26) | (reg(r.rstrip(")")) << 21) | (reg(a[0]) << 16) | (int(off or "0", 0) & 0xFFFF)
        elif m in ("beq", "bne"):
            off = (labels[a[2]] - (pc + 4)) >> 2
            w = ((4 if m == "beq" else 5) << 26) | (reg(a[0]) << 21) | (reg(a[1]) << 16) | (off & 0xFFFF)
        elif m in ("j", "jal"):
            w = ((2 if m == "j" else 3) << 26) | ((labels[a[0]] >> 2) & 0x3FFFFFF)
        elif m == "jr":
            w = (reg(a[0]) << 21) | 8
        else:
            raise SystemExit("assembler: instrucao nao suportada: " + m)
        words.append(w & 0xFFFFFFFF)
    return words, labels

# ------------------------------------------------------------------ recompilador
def sx16(v):
    return v - 0x10000 if v & 0x8000 else v

def u32(v):
    return v & 0xFFFFFFFF

WARN = {}

def warn(what):
    WARN[what] = WARN.get(what, 0) + 1

def setr(d, e):
    return "" if d == 0 else "R[%d] = %s;" % (d, e)

def stmt(w):
    """C de uma instrucao que nao e desvio."""
    op = w >> 26
    rs, rt, rd = (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
    sh, fn = (w >> 6) & 31, w & 63
    si = sx16(w & 0xFFFF)
    ui = w & 0xFFFF
    A = "R[%d]" % rs
    B = "R[%d]" % rt
    off = "(R[%d] + %du)" % (rs, u32(si)) if si else "R[%d]" % rs
    if op == 0:
        if fn == 0:   return "" if w == 0 else setr(rd, "%s << %d" % (B, sh))
        if fn == 2:   return setr(rd, "%s >> %d" % (B, sh))
        if fn == 3:   return setr(rd, "(uint32_t)((int32_t)%s >> %d)" % (B, sh))
        if fn == 4:   return setr(rd, "%s << (%s & 31)" % (B, A))
        if fn == 6:   return setr(rd, "%s >> (%s & 31)" % (B, A))
        if fn == 7:   return setr(rd, "(uint32_t)((int32_t)%s >> (%s & 31))" % (B, A))
        if fn == 16:  return setr(rd, "HI")
        if fn == 17:  return "HI = %s;" % A
        if fn == 18:  return setr(rd, "LO")
        if fn == 19:  return "LO = %s;" % A
        if fn == 24:  return "m64 = (int64_t)(int32_t)%s * (int64_t)(int32_t)%s; LO = (uint32_t)m64; HI = (uint32_t)(m64 >> 32);" % (A, B)
        if fn == 25:  return "u64 = (uint64_t)%s * (uint64_t)%s; LO = (uint32_t)u64; HI = (uint32_t)(u64 >> 32);" % (A, B)
        if fn == 26:
            return ("{ int32_t n = (int32_t)%s, d = (int32_t)%s; "
                    "if (d == 0) { LO = (n >= 0) ? 0xFFFFFFFFu : 1u; HI = (uint32_t)n; } "
                    "else if ((uint32_t)n == 0x80000000u && d == -1) { LO = 0x80000000u; HI = 0; } "
                    "else { LO = (uint32_t)(n / d); HI = (uint32_t)(n %% d); } }") % (A, B)
        if fn == 27:
            return ("{ uint32_t n = %s, d = %s; "
                    "if (d == 0) { LO = 0xFFFFFFFFu; HI = n; } else { LO = n / d; HI = n %% d; } }") % (A, B)
        if fn in (32, 33): return setr(rd, "%s + %s" % (A, B))
        if fn in (34, 35): return setr(rd, "%s - %s" % (A, B))
        if fn == 36:  return setr(rd, "%s & %s" % (A, B))
        if fn == 37:  return setr(rd, "%s | %s" % (A, B))
        if fn == 38:  return setr(rd, "%s ^ %s" % (A, B))
        if fn == 39:  return setr(rd, "~(%s | %s)" % (A, B))
        if fn == 42:  return setr(rd, "(uint32_t)((int32_t)%s < (int32_t)%s)" % (A, B))
        if fn == 43:  return setr(rd, "(uint32_t)(%s < %s)" % (A, B))
        if fn in (12, 13):
            warn("syscall/break"); return "return; /* syscall/break */"
        warn("SPECIAL fn=%d" % fn); return "/* ?? */"
    if op in (8, 9):  return setr(rt, "%s + %du" % (A, u32(si)))
    if op == 10:      return setr(rt, "(uint32_t)((int32_t)%s < %d)" % (A, si))
    if op == 11:      return setr(rt, "(uint32_t)(%s < %du)" % (A, u32(si)))
    if op == 12:      return setr(rt, "%s & %du" % (A, ui))
    if op == 13:      return setr(rt, "%s | %du" % (A, ui))
    if op == 14:      return setr(rt, "%s ^ %du" % (A, ui))
    if op == 15:      return setr(rt, "%du" % (ui << 16))
    if op == 32:      return setr(rt, "(uint32_t)(int32_t)(int8_t)ps1_r8(%s)" % off) if rt else "(void)ps1_r8(%s);" % off
    if op == 33:      return setr(rt, "(uint32_t)(int32_t)(int16_t)ps1_r16(%s)" % off) if rt else "(void)ps1_r16(%s);" % off
    if op == 35:      return setr(rt, "ps1_r32(%s)" % off) if rt else "(void)ps1_r32(%s);" % off
    if op == 36:      return setr(rt, "ps1_r8(%s)" % off) if rt else "(void)ps1_r8(%s);" % off
    if op == 37:      return setr(rt, "ps1_r16(%s)" % off) if rt else "(void)ps1_r16(%s);" % off
    if op == 34:
        if rt == 0: return ""
        return ("{ uint32_t a = %s, v = ps1_r32(a & ~3u), r = R[%d]; "
                "switch (a & 3) { case 0: r = (r & 0x00FFFFFFu) | (v << 24); break; "
                "case 1: r = (r & 0x0000FFFFu) | (v << 16); break; "
                "case 2: r = (r & 0x000000FFu) | (v << 8); break; default: r = v; break; } R[%d] = r; }") % (off, rt, rt)
    if op == 38:
        if rt == 0: return ""
        return ("{ uint32_t a = %s, v = ps1_r32(a & ~3u), r = R[%d]; "
                "switch (a & 3) { case 0: r = v; break; "
                "case 1: r = (r & 0xFF000000u) | (v >> 8); break; "
                "case 2: r = (r & 0xFFFF0000u) | (v >> 16); break; default: r = (r & 0xFFFFFF00u) | (v >> 24); break; } R[%d] = r; }") % (off, rt, rt)
    if op == 42:
        return ("{ uint32_t a = %s, v = ps1_r32(a & ~3u), r = R[%d]; "
                "switch (a & 3) { case 0: v = (v & 0xFFFFFF00u) | (r >> 24); break; "
                "case 1: v = (v & 0xFFFF0000u) | (r >> 16); break; "
                "case 2: v = (v & 0xFF000000u) | (r >> 8); break; default: v = r; break; } ps1_w32(a & ~3u, v); }") % (off, rt)
    if op == 46:
        return ("{ uint32_t a = %s, v = ps1_r32(a & ~3u), r = R[%d]; "
                "switch (a & 3) { case 0: v = r; break; "
                "case 1: v = (v & 0x000000FFu) | (r << 8); break; "
                "case 2: v = (v & 0x0000FFFFu) | (r << 16); break; default: v = (v & 0x00FFFFFFu) | (r << 24); break; } ps1_w32(a & ~3u, v); }") % (off, rt)
    if op == 16:
        if rs == 0:  return setr(rt, "C0[%d]" % rd)
        if rs == 4:  return "C0[%d] = %s;" % (rd, B)
        if w & 0x3F == 0x10:
            return "C0[12] = (C0[12] & 0xFFFFFFF0u) | ((C0[12] >> 2) & 0xFu);"
        warn("COP0 rs=%d" % rs); return "/* cop0 */"
    if op in (18, 50, 58):
        warn("GTE/COP2"); return "/* GTE nao suportado */"
    if op == 40:      return "ps1_w8(%s, (uint8_t)%s);" % (off, B)
    if op == 41:      return "ps1_w16(%s, (uint16_t)%s);" % (off, B)
    if op == 43:      return "ps1_w32(%s, %s);" % (off, B)
    warn("opcode %d" % op)
    return "/* opcode %d nao suportado */" % op

def is_branch(w):
    op, fn = w >> 26, w & 63
    return op in (1, 2, 3, 4, 5, 6, 7) or (op == 0 and fn in (8, 9))

def is_cond(w):
    return (w >> 26) in (1, 4, 5, 6, 7)

def recompile(words, base, entries, all_labels=False):
    n = len(words)
    end = base + 4 * n
    inr = lambda a: base <= a < end and a % 4 == 0
    targets = set(entries)
    delay = set()
    for i, w in enumerate(words):
        pc = base + 4 * i
        op, fn = w >> 26, w & 63
        if is_branch(w):
            delay.add(i + 1)
        if op in (1, 4, 5, 6, 7):
            targets.add(pc + 4 + (sx16(w & 0xFFFF) << 2)); targets.add(pc + 8)
        elif op in (2, 3):
            targets.add(((pc + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2)); targets.add(pc + 8)
        elif op == 0 and fn == 9:
            targets.add(pc + 8)
    if all_labels:
        targets |= set(range(base, end, 4))
    targets = {t for t in targets if inr(t)}

    def go(a):
        if a in targets:
            return "goto L_%08X;" % a
        return "{ pc = 0x%08Xu; goto dispatch; }" % a

    def slot_code(i):
        return stmt(words[i]) if i < n else ""

    out = []
    for i, w in enumerate(words):
        pc = base + 4 * i
        if i in delay and pc not in targets:
            continue
        if pc in targets:
            out.append("L_%08X:" % pc)
        if not is_branch(w):
            s = stmt(w)
            if s:
                out.append("  " + s)
            continue
        op, fn = w >> 26, w & 63
        rs, rt, rd = (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
        slot = slot_code(i + 1)
        if is_cond(w):
            tgt = pc + 4 + (sx16(w & 0xFFFF) << 2)
            A, B = "R[%d]" % rs, "R[%d]" % rt
            if op == 4:   cond = "%s == %s" % (A, B)
            elif op == 5: cond = "%s != %s" % (A, B)
            elif op == 6: cond = "(int32_t)%s <= 0" % A
            elif op == 7: cond = "(int32_t)%s > 0" % A
            else:
                cond = "(int32_t)%s < 0" % A if rt in (0, 16) else "(int32_t)%s >= 0" % A
            out.append("  { c = (uint32_t)(%s);" % cond)
            if op == 1 and rt in (16, 17):
                out.append("    R[31] = 0x%08Xu;" % u32(pc + 8))
            if slot:
                out.append("    " + slot)
            out.append("    if (c) %s }" % go(tgt))
            if (i + 1) in delay and (pc + 4) in targets:
                out.append("  " + go(pc + 8))
        elif op in (2, 3):
            tgt = ((pc + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
            out.append("  {")
            if op == 3:
                out.append("    R[31] = 0x%08Xu;" % u32(pc + 8))
            if slot:
                out.append("    " + slot)
            out.append("    " + go(tgt) + " }")
        else:  # jr / jalr
            out.append("  { tgt = R[%d];" % rs)
            if fn == 9 and rd:
                out.append("    R[%d] = 0x%08Xu;" % (rd, u32(pc + 8)))
            if slot:
                out.append("    " + slot)
            out.append("    pc = tgt; goto dispatch; }")

    cases = "\n".join("  case 0x%08Xu: goto L_%08X;" % (t, t) for t in sorted(targets))
    code = """// gerado por mipsrecomp.py - NAO EDITE
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
%s
  case RET_ADDR: return;
  default: printf("mips: salto para %%08X sem codigo\\n", (unsigned)pc); return;
  }
%s
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
  run(0x%08Xu);
}
""" % (cases, "\n".join(out), entries[0])
    return code

HEADER = """#ifndef MIPS_GEN_H
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
"""

def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    outdir = args[0] if args else "ps1rt"
    base = 0x80010000
    fn = os.path.join(os.path.dirname(os.path.abspath(__file__)), "selftest.asm")
    asm = open(fn).read()
    words, labels = assemble(asm, base)
    entries = [labels["main"]] + [v for k, v in labels.items() if k.startswith("fn_")]
    code = recompile(words, base, entries, "--all" in sys.argv)
    with open(os.path.join(outdir, "source", "mips_gen.cpp"), "w") as f:
        f.write(code)
    with open(os.path.join(outdir, "include", "mips_gen.h"), "w") as f:
        f.write(HEADER)
    print("%d instrucoes MIPS -> %s/source/mips_gen.cpp" % (len(words), outdir))
    for k, v in WARN.items():
        print("aviso: %s (x%d)" % (k, v))

if __name__ == "__main__":
    main()

# Ganchos de Transição Híbrida Automática

def translate_branch_target(target_address, is_link=False, reg_source=None):
    if reg_source is not None:
        if is_link:
            return f"ps1_cpu_regs.ra = current_pc + 8; ps1_cpu_regs.pc = ps1_cpu_regs.gpr[{reg_source}]; if (ps1_cpu_regs.pc >= 0x80010000 && ps1_cpu_regs.pc <= 0x8009FFFF) {{ execute_recompiled_block(ps1_cpu_regs.pc); }} else {{ ps1_cpu_regs.interpreter_mode = true; return; }}"
        else:
            return f"ps1_cpu_regs.pc = ps1_cpu_regs.gpr[{reg_source}]; if (ps1_cpu_regs.pc >= 0x80010000 && ps1_cpu_regs.pc <= 0x8009FFFF) {{ goto *dispatch_table[ps1_cpu_regs.pc & 0xFFFFF]; }} else {{ ps1_cpu_regs.interpreter_mode = true; return; }}"
    if 0x80010000 <= target_address <= 0x8009FFFF:
        if is_link: return f"func_{target_address:08X}();"
        else: return f"goto label_{target_address:08X};"
    elif 0x800A0000 <= target_address <= 0x801FFFFF:
        if is_link: return f"ps1_cpu_regs.ra = current_pc + 8; ps1_cpu_regs.pc = {hex(target_address)}; ps1_cpu_regs.interpreter_mode = true; return;"
        else: return f"ps1_cpu_regs.pc = {hex(target_address)}; ps1_cpu_regs.interpreter_mode = true; return;"
    else: return f"ps1_trigger_bus_error({hex(target_address)});"
