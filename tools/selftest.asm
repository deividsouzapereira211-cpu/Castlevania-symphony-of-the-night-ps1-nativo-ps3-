# autoteste MIPS: cada teste desenha um quadrado verde (passou) ou vermelho (falhou)
main:
    addiu $sp, $sp, -32
    sw $ra, 28($sp)
    sw $s0, 24($sp)
    sw $s1, 20($sp)
    sw $s2, 16($sp)
    sw $s3, 12($sp)
    move $s1, $a0
    move $s2, $a1
    move $s3, $a2
    li $s0, 0x80100000
    lui $t0, 0x1F80
    ori $t0, $t0, 0x1810
    lui $t1, 0xE300
    sw $t1, 0($t0)
    lui $t1, 0xE403
    ori $t1, $t1, 0xBD3F
    sw $t1, 0($t0)
    lui $t1, 0xE500
    sw $t1, 0($t0)
    move $a0, $zero
    move $a1, $zero
    li $a2, 0x00F00140
    li $a3, 0x00402010
    jal put_rect
    nop
# 0: carga nao alinhada (lwr+lwl)
    li $t0, 0x44332211
    sw $t0, 0($s0)
    li $t0, 0x88776655
    sw $t0, 4($s0)
    move $v0, $zero
    lwr $v0, 1($s0)
    lwl $v0, 4($s0)
    move $a1, $v0
    li $a2, 0x55443322
    jal check
    addiu $a0, $zero, 0
# 1: gravacao nao alinhada (swr+swl), palavra em +8
    sw $zero, 8($s0)
    sw $zero, 12($s0)
    li $t0, 0xAABBCCDD
    swr $t0, 9($s0)
    swl $t0, 12($s0)
    lw $a1, 8($s0)
    li $a2, 0xBBCCDD00
    jal check
    addiu $a0, $zero, 1
# 2: palavra em +12
    lw $a1, 12($s0)
    li $a2, 0x000000AA
    jal check
    addiu $a0, $zero, 2
# 3: mult (LO)
    li $t0, -3
    li $t1, 5
    mult $t0, $t1
    mflo $a1
    li $a2, 0xFFFFFFF1
    jal check
    addiu $a0, $zero, 3
# 4: mult (HI)
    mfhi $a1
    li $a2, 0xFFFFFFFF
    jal check
    addiu $a0, $zero, 4
# 5: multu (HI)
    li $t0, 0xFFFFFFFF
    li $t1, 2
    multu $t0, $t1
    mfhi $a1
    li $a2, 1
    jal check
    addiu $a0, $zero, 5
# 6: div (quociente)
    li $t0, 7
    li $t1, -2
    div $t0, $t1
    mflo $a1
    li $a2, 0xFFFFFFFD
    jal check
    addiu $a0, $zero, 6
# 7: div (resto)
    mfhi $a1
    li $a2, 1
    jal check
    addiu $a0, $zero, 7
# 8: divu
    li $t0, 0xFFFFFFFE
    li $t1, 3
    divu $t0, $t1
    mflo $a1
    li $a2, 0x55555554
    jal check
    addiu $a0, $zero, 8
# 9: div por zero
    li $t0, 5
    div $t0, $zero
    mflo $a1
    li $a2, 0xFFFFFFFF
    jal check
    addiu $a0, $zero, 9
# 10: sra
    li $t0, 0x80000000
    sra $a1, $t0, 4
    li $a2, 0xF8000000
    jal check
    addiu $a0, $zero, 10
# 11: srav (quantidade mascarada em 5 bits)
    li $t0, 0x80000000
    li $t1, 36
    srav $a1, $t0, $t1
    li $a2, 0xF8000000
    jal check
    addiu $a0, $zero, 11
# 12: slt e sltu
    li $t0, -1
    li $t1, 1
    slt $t2, $t0, $t1
    sltu $t3, $t0, $t1
    sll $t2, $t2, 1
    or $a1, $t2, $t3
    li $a2, 2
    jal check
    addiu $a0, $zero, 12
# 13: lb, lbu, lh, lhu
    li $t0, 0x00008080
    sw $t0, 16($s0)
    lb $t1, 16($s0)
    lbu $t2, 16($s0)
    lh $t3, 16($s0)
    lhu $t4, 16($s0)
    xor $a1, $t1, $t2
    xor $a1, $a1, $t3
    xor $a1, $a1, $t4
    li $a2, 0x0000FF00
    jal check
    addiu $a0, $zero, 13
# 14: desvios condicionais
    move $v0, $zero
    li $t0, -1
    bltz $t0, t14a
    nop
    ori $v0, $v0, 1
t14a:
    bgez $t0, t14b
    nop
    ori $v0, $v0, 2
t14b:
    blez $t0, t14c
    nop
    ori $v0, $v0, 4
t14c:
    bgtz $t0, t14d
    nop
    ori $v0, $v0, 8
t14d:
    move $a1, $v0
    li $a2, 10
    jal check
    addiu $a0, $zero, 14
# 15: delay slot (condicao avaliada antes do slot)
    li $t0, 1
    move $v0, $zero
    bne $t0, $zero, t15
    move $t0, $zero
    ori $v0, $v0, 1
t15:
    ori $v0, $v0, 2
    addu $a1, $v0, $t0
    li $a2, 2
    jal check
    addiu $a0, $zero, 15
# 16: jalr (ponteiro de funcao) com delay slot
    la $t0, fn_leaf
    jalr $t0
    addiu $v0, $zero, 7
    move $a1, $v0
    li $a2, 8
    jal check
    addiu $a0, $zero, 16
# 17: mtc0 / mfc0
    li $t0, 0x12345678
    mtc0 $t0, $13
    mfc0 $a1, $13
    li $a2, 0x12345678
    jal check
    addiu $a0, $zero, 17
# 18: ordem dos bytes (little-endian)
    addiu $t0, $zero, 1
    sb $t0, 20($s0)
    addiu $t0, $zero, 2
    sb $t0, 21($s0)
    addiu $t0, $zero, 3
    sb $t0, 22($s0)
    addiu $t0, $zero, 4
    sb $t0, 23($s0)
    lw $a1, 20($s0)
    li $a2, 0x04030201
    jal check
    addiu $a0, $zero, 18
# 19: nor
    nor $a1, $zero, $zero
    li $a2, 0xFFFFFFFF
    jal check
    addiu $a0, $zero, 19
# quadrado do jogador
    move $a0, $s1
    move $a1, $s2
    li $a2, 0x00100010
    jal put_rect
    move $a3, $s3
    lw $s3, 12($sp)
    lw $s2, 16($sp)
    lw $s1, 20($sp)
    lw $s0, 24($sp)
    lw $ra, 28($sp)
    addiu $sp, $sp, 32
    jr $ra
    nop
fn_leaf:
    addiu $v0, $v0, 1
    jr $ra
    nop
check:
    xor $t8, $a1, $a2
    sltiu $t8, $t8, 1
    subu $t9, $zero, $t8
    andi $t7, $t9, 0xFF00
    nor $t9, $t9, $zero
    andi $t9, $t9, 0xFF
    or $a3, $t7, $t9
    andi $t0, $a0, 15
    sll $t1, $t0, 4
    sll $t2, $t0, 1
    addu $t1, $t1, $t2
    addiu $t3, $t1, 8
    srl $t4, $a0, 4
    sll $t5, $t4, 4
    sll $t6, $t4, 3
    addu $t5, $t5, $t6
    addiu $t5, $t5, 8
    move $a0, $t3
    move $a1, $t5
    li $a2, 0x00100010
    j put_rect
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
