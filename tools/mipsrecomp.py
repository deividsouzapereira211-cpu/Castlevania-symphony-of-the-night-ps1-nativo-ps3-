#!/usr/bin/env python3
# mipsrecomp.py - recompilador estatico MIPS R3000 (PS1) -> C++  (v1)
# Inclui um mini assembler e um programa de teste que desenha pelo GPU.
# Uso: python3 tools/mipsrecomp.py ps1rt
import sys, os

REGS = ["zero","at","v0","v1","a0","a1","a2","a3","t0","t1","t2","t3","t4","t5","t6","t7",
        "s0","s1","s2","s3","s4","s5","s6","s7","t8","t9","k0","k1","gp","sp","fp","ra"]
RN = {n: i for i, n in enumerate(REGS)}

def reg(s):
    s = s.strip().lstrip("$")
    return RN[s] if s in RN else int(s)

RTYPE = {"addu":0x21,"subu":0x23,"and":0x24,"or":0x25,"xor":0x26,"nor":0x27,"slt":0x2A,"sltu":0x2B}
ITYPE = {"addiu":9,"slti":10,"sltiu":11,"andi":12,"ori":13,"xori":14}
MEM = {"lb":0x20,"lh":0x21,"lw":0x23,"lbu":0x24,"lhu":0x25,"sb":0x28,"sh":0x29,"sw":0x2B}

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
    words = []
    for i, (m, a) in enumerate(items):
        pc = base + 4 * i
        if m in RTYPE:
            w = (reg(a[1]) << 21) | (reg(a[2]) << 16) | (reg(a[0]) << 11) | RTYPE[m]
        elif m in ("sll", "srl", "sra"):
            fn = {"sll": 0, "srl": 2, "sra": 3}[m]
            w = (reg(a[1]) << 16) | (reg(a[0]) << 11) | (int(a[2], 0) << 6) | fn
        elif m in ITYPE:
            w = (ITYPE[m] << 26) | (reg(a[1]) << 21) | (reg(a[0]) << 16) | (int(a[2], 0) & 0xFFFF)
        elif m == "lui":
            w = (15 << 26) | (reg(a[0]) << 16) | (int(a[1], 0) & 0xFFFF)
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

TEST_ASM = """
main:
    addiu $sp, $sp, -32
    sw $ra, 28($sp)
    sw $s0, 24($sp)
    sw $s1, 20($sp)
    sw $s2, 16($sp)
    sw $s3, 12($sp)
    move $s0, $a0
    move $s1, $a1
    move $s2, $a2
    lui $t0, 0x1F80
    ori $t0, $t0, 0x1810
    lui $t1, 0xE300
    sw $t1, 0($t0)
    lui $t1, 0xE403
    ori $t1, $t1, 0xBD3F
    sw $t1, 0($t0)
    lui $t1, 0xE500
    sw $t1, 0($t0)
    move $s3, $zero
bars:
    sll $t2, $s3, 5
    sll $t3, $s3, 3
    addu $a0, $t2, $t3
    move $a1, $zero
    li $a2, 0x00F00028
    li $t5, 255
    subu $t5, $t5, $t2
    sll $t5, $t5, 8
    or $a3, $t2, $t5
    lui $t6, 0x0080
    or $a3, $a3, $t6
    jal put_rect
    nop
    addiu $s3, $s3, 1
    slti $t7, $s3, 8
    bne $t7, $zero, bars
    nop
    move $a0, $s0
    move $a1, $s1
    li $a2, 0x00100010
    jal put_rect
    move $a3, $s2
    lw $s3, 12($sp)
    lw $s2, 16($sp)
    lw $s1, 20($sp)
    lw $s0, 24($sp)
    lw $ra, 28($sp)
    addiu $sp, $sp, 32
    jr $ra
    nop
put_rect:
    lui $t0, 0x1F80
    ori $t0, $t0, 0x1810
    lui $t1, 0x6000
    or $t1, $t1, $a3
    sw $t1, 0($t0)
    sll $t2, $a1, 16
    andi $t3, $a0, 0x7FF
    or $t2, $t2, $t3
    sw $t2, 0($t0)
    sw $a2, 0($t0)
    jr $ra
    nop
"""

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

def recompile(words, base, entries):
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
        else:
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
%s
  case RET_ADDR: return;
  default: printf("mips: salto para %%08X sem codigo\\n", (unsigned)pc); return;
  }
%s
  goto dispatch;
}

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
#endif
"""

def main():
    outdir = sys.argv[1] if len(sys.argv) > 1 else "ps1rt"
    base = 0x80010000
    words, labels = assemble(TEST_ASM, base)
    code = recompile(words, base, [labels["main"]])
    with open(os.path.join(outdir, "source", "mips_gen.cpp"), "w") as f:
        f.write(code)
    with open(os.path.join(outdir, "include", "mips_gen.h"), "w") as f:
        f.write(HEADER)
    print("%d instrucoes MIPS -> %s/source/mips_gen.cpp" % (len(words), outdir))
    for k, v in WARN.items():
        print("aviso: %s (x%d)" % (k, v))

if __name__ == "__main__":
    main()
