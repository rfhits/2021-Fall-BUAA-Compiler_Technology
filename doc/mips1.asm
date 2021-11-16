.data
.text

sgt $t1, $t2, 0

# FUNC_BEGIN sum2  
addi $sp, $sp, -32
j main
sum2 :

# ADD #Tmp0 a_1 b_1
lw $s0, 0($sp)
lw $s1, 4($sp)
add $t0, $s0, $s1

# RET #Tmp0  
move $v0, $t0
jr $ra

# FUNC_END sum2  
jr $ra

# FUNC_BEGIN sum3  
sum3 :

# ADD #Tmp1 a_5 b_5
lw $s0, 0($sp)
lw $s1, 4($sp)
add $t0, $s0, $s1

# ADD #Tmp2 #Tmp1 c_5
lw $s2, 8($sp)
add $t1, $t0, $s2

# RET #Tmp2  
move $v0, $t1
jr $ra

# FUNC_END sum3  
jr $ra

# FUNC_BEGIN main  
main :

# ADD a_10 1 0
lw $s0, 0($sp)
add $s0, $zero, 1

# ADD b_10 2 0
lw $s1, 4($sp)
add $s1, $zero, 2

# ADD c_10 3 0
lw $s2, 8($sp)
add $s2, $zero, 3

# ADD d_10 4 0
lw $s3, 12($sp)
add $s3, $zero, 4

# PREPARE_CALL sum2  
sub $sp, $sp, 112

# PREPARE_CALL sum2  
sub $sp, $sp, 112

# PUSH_VAL a_10 0 
sw $s0, 0($sp)

# PUSH_VAL b_10 1 
sw $s1, 4($sp)

# CALL sum2  
sw $ra, 12($sp)
sw $s0, 36($sp)
sw $s1, 40($sp)
sw $s2, 44($sp)
sw $s3, 48($sp)
jal sum2
lw $ra, 12($sp)
lw $s0, 36($sp)
lw $s1, 40($sp)
lw $s2, 44($sp)
lw $s3, 48($sp)
add $sp, $sp, 112

# ADD #Tmp3 %RET 0
add $t0, $v0, 0

# PREPARE_CALL sum2  
sub $sp, $sp, 112

# PUSH_VAL a_10 0 
sw $s0, 0($sp)

# PUSH_VAL b_10 1 
sw $s1, 4($sp)

# CALL sum2  
sw $ra, 12($sp)
sw $s0, 36($sp)
sw $s1, 40($sp)
sw $s2, 44($sp)
sw $s3, 48($sp)
sw $t0, 68($sp)
jal sum2
lw $ra, 12($sp)
lw $s0, 36($sp)
lw $s1, 40($sp)
lw $s2, 44($sp)
lw $s3, 48($sp)
lw $t0, 68($sp)
add $sp, $sp, 112

# ADD #Tmp4 %RET 0
add $t1, $v0, 0

# PUSH_VAL #Tmp3 0 
sw $t0, 0($sp)

# PUSH_VAL #Tmp4 1 
sw $t1, 4($sp)

# CALL sum2  
sw $ra, 12($sp)
sw $s0, 36($sp)
sw $s1, 40($sp)
sw $s2, 44($sp)
sw $s3, 48($sp)
jal sum2
lw $ra, 12($sp)
lw $s0, 36($sp)
lw $s1, 40($sp)
lw $s2, 44($sp)
lw $s3, 48($sp)
add $sp, $sp, 112

# ADD #Tmp5 %RET 0
add $t2, $v0, 0

# ADD e_11 #Tmp5 0
lw $s4, 16($sp)
add $s4, $t2, 0

# PRINT e_11 int 
move $a0, $s4
li $v0, 1
syscall

# RET 0  
add $v0, $zero, 0
li $v0, 10
syscall

# FUNC_END main  
