.data
	str_0:	.asciiz ", "
	str_1:	.asciiz "\n"
.text

# ADD d_1 4 0
lw $s0, 0($gp)
add $s0, $zero, 4

# FUNC_BEGIN hhh  
addi $sp, $sp, -76
j main
hhh :

# MUL #Tmp0 i_3 i_3
lw $s0, 0($sp)
mul $t0, $s0, $s0

# ADD c_4 #Tmp0 0
lw $s1, 4($sp)
add $s1, $t0, 0

# EQ #Tmp3 i_3 1
seq $t1, $s0, 1

# ADD #Tmp2 #Tmp3 0
add $t2, $t1, 0

# LAnd_End_Label_0:
sw $s0, 0($sp)
sw $s1, 4($sp)
sw $t2, 16($sp)
LAnd_End_Label_0:

# ADD #Tmp1 #Tmp2 0
lw $t3, 16($sp)
add $t2, $t3, 0

# Cond_End_Label_0:
sw $t2, 12($sp)
Cond_End_Label_0:

# BEQ Label_0 #Tmp1 0
lw $t2, 12($sp)
sw $t2, 12($sp)
beq $t2, $0, Label_0

# RET 1  
add $v0, $zero, 1
jr $ra

# Label_0:
Label_0:

# EQ #Tmp6 i_3 2
lw $s0, 0($sp)
seq $t2, $s0, 2

# ADD #Tmp5 #Tmp6 0
add $t4, $t2, 0

# LAnd_End_Label_1:
sw $s0, 0($sp)
sw $t4, 28($sp)
LAnd_End_Label_1:

# ADD #Tmp4 #Tmp5 0
lw $t5, 28($sp)
add $t4, $t5, 0

# Cond_End_Label_1:
sw $t4, 24($sp)
Cond_End_Label_1:

# BEQ Label_1 #Tmp4 0
lw $t4, 24($sp)
sw $t4, 24($sp)
beq $t4, $0, Label_1

# RET 2  
add $v0, $zero, 2
jr $ra

# Label_1:
Label_1:

# MUL #Tmp7 c_4 c_4
lw $s0, 4($sp)
mul $t4, $s0, $s0

# MOD #Tmp8 #Tmp7 10
add $a1, $zero, 10
div $t4, $a1
mfhi $t6

# ADD c_4 #Tmp8 0
add $s0, $t6, 0

# SUB #Tmp9 i_3 1
lw $s1, 0($sp)
sub $t7, $s1, 1

# PREPARE_CALL hhh  
sub $sp, $sp, 164

# PUSH_VAL #Tmp9 0 
sw $t7, 0($sp)

# CALL hhh  
sw $ra, 64($sp)
sw $s0, 88($sp)
sw $s1, 92($sp)
jal hhh
lw $ra, 64($sp)
lw $s0, 88($sp)
lw $s1, 92($sp)
add $sp, $sp, 164

# ADD #Tmp10 %RET 0
add $t0, $v0, 0

# SUB #Tmp11 i_3 2
sub $t1, $s1, 2

# PREPARE_CALL hhh  
sub $sp, $sp, 164

# PUSH_VAL #Tmp11 0 
sw $t1, 0($sp)

# CALL hhh  
sw $ra, 64($sp)
sw $s0, 88($sp)
sw $s1, 92($sp)
sw $t0, 120($sp)
jal hhh
lw $ra, 64($sp)
lw $s0, 88($sp)
lw $s1, 92($sp)
lw $t0, 120($sp)
add $sp, $sp, 164

# ADD #Tmp12 %RET 0
add $t3, $v0, 0

# ADD #Tmp13 #Tmp10 #Tmp12
add $t2, $t0, $t3

# RET #Tmp13  
move $v0, $t2
jr $ra

# FUNC_END hhh  
jr $ra

# FUNC_BEGIN main  
main :

# ADD i_16 2 0
lw $s1, 0($sp)
add $s1, $zero, 2

# ADD j_16 5 0
lw $s2, 4($sp)
add $s2, $zero, 5

# GETINT i_16  
li $v0, 5
syscall
move $s1, $v0

# GETINT j_16  
li $v0, 5
syscall
move $s2, $v0

# PREPARE_CALL hhh  
sub $sp, $sp, 164

# PUSH_VAL 3 0 
add $a0, $zero, 3
sw $a0, 0($sp)

# CALL hhh  
sw $s0, 0($gp)
sw $ra, 64($sp)
sw $s1, 92($sp)
sw $s2, 96($sp)
jal hhh
lw $ra, 64($sp)
lw $s1, 92($sp)
lw $s2, 96($sp)
add $sp, $sp, 164

# ADD #Tmp14 %RET 0
add $t0, $v0, 0

# PREPARE_CALL hhh  
sub $sp, $sp, 164

# PUSH_VAL #Tmp14 0 
sw $t0, 0($sp)

# CALL hhh  
sw $ra, 64($sp)
sw $s1, 92($sp)
sw $s2, 96($sp)
jal hhh
lw $ra, 64($sp)
lw $s1, 92($sp)
lw $s2, 96($sp)
add $sp, $sp, 164

# ADD #Tmp15 %RET 0
add $t1, $v0, 0

# SUB #Tmp16 41440 #Tmp15
add $a0, $zero, 41440
sub $t2, $a0, $t1

# ADD #Tmp17 #Tmp16 -10091
add $t3, $t2, -10091

# ADD j_16 #Tmp17 0
add $s2, $t3, 0

# ADD k_20 5 0
lw $s0, 24($sp)
add $s0, $zero, 5

# ADD n_21 10 0
lw $s3, 28($sp)
add $s3, $zero, 10

# While_Begin_Label_0:
sw $s0, 24($sp)
sw $s1, 0($sp)
sw $s2, 4($sp)
sw $s3, 28($sp)
While_Begin_Label_0:

# MUL #Tmp20 k_20 k_20
lw $s0, 24($sp)
mul $t4, $s0, $s0

# MUL #Tmp21 #Tmp20 k_20
mul $t5, $t4, $s0

# MUL #Tmp22 #Tmp21 k_20
mul $t6, $t5, $s0

# MUL #Tmp23 #Tmp22 k_20
mul $t7, $t6, $s0

# MUL #Tmp24 #Tmp23 k_20
mul $t0, $t7, $s0

# LSS #Tmp25 n_21 #Tmp24
lw $s1, 28($sp)
slt $t1, $s1, $t0

# ADD #Tmp19 #Tmp25 0
add $t2, $t1, 0

# LAnd_End_Label_2:
sw $s0, 24($sp)
sw $s1, 28($sp)
sw $t2, 36($sp)
LAnd_End_Label_2:

# ADD #Tmp18 #Tmp19 0
lw $t3, 36($sp)
add $t2, $t3, 0

# Cond_End_Label_2:
sw $t2, 32($sp)
Cond_End_Label_2:

# BEQ While_End_Label_0 #Tmp18 0
lw $t2, 32($sp)
sw $t2, 32($sp)
beq $t2, $0, While_End_Label_0

# MUL #Tmp26 d_1 d_1
lw $s0, 0($gp)
mul $t2, $s0, $s0

# MOD #Tmp27 #Tmp26 10000
add $a1, $zero, 10000
div $t2, $a1
mfhi $t4

# ADD d_1 #Tmp27 0
add $s0, $t4, 0

# ADD #Tmp28 n_21 1
lw $s1, 28($sp)
add $t5, $s1, 1

# ADD n_21 #Tmp28 0
add $s1, $t5, 0

# JUMP While_Begin_Label_0  
sw $s0, 0($gp)
sw $s1, 28($sp)
j While_Begin_Label_0

# While_End_Label_0:
While_End_Label_0:

# PRINT i_16 int 
lw $a0, 0($sp)
li $v0, 1
syscall

# PRINT ,  str 
la $a0, str_0
li $v0, 4
syscall

# PRINT j_16 int 
lw $a0, 4($sp)
li $v0, 1
syscall

# PRINT ,  str 
la $a0, str_0
li $v0, 4
syscall

# PRINT k_20 int 
lw $a0, 24($sp)
li $v0, 1
syscall

# PRINT \n str 
la $a0, str_1
li $v0, 4
syscall

# RET 0  
add $v0, $zero, 0
li $v0, 10
syscall

# FUNC_END main  
