.data
	str_0:	.asciiz "a: "
.text

# FUNC_BEGIN fibo  
addi $sp, $sp, -64
j main
fibo :

# SUB #Tmp0 n_1 1
lw $s0, 0($sp)
sub $t0, $s0, 1

# PREPARE_CALL fibo  
sub $sp, $sp, 124

# PUSH_VAL #Tmp0 0 
sw $t0, 0($sp)

# CALL fibo  
sw $ra, 24($sp)
sw $s0, 48($sp)
jal fibo
lw $ra, 24($sp)
lw $s0, 48($sp)
add $sp, $sp, 124

# ADD #Tmp1 %RET 0
add $t1, $v0, 0

# SUB #Tmp2 n_1 2
sub $t2, $s0, 2

# PREPARE_CALL fibo  
sub $sp, $sp, 124

# PUSH_VAL #Tmp2 0 
sw $t2, 0($sp)

# CALL fibo  
sw $ra, 24($sp)
sw $s0, 48($sp)
sw $t1, 84($sp)
jal fibo
lw $ra, 24($sp)
lw $s0, 48($sp)
lw $t1, 84($sp)
add $sp, $sp, 124

# ADD #Tmp3 %RET 0
add $t3, $v0, 0

# ADD #Tmp4 #Tmp1 #Tmp3
add $t4, $t1, $t3

# RET #Tmp4  
move $v0, $t4
jr $ra

# FUNC_END fibo  
jr $ra

# FUNC_BEGIN sum  
sum :

# ADD #Tmp5 a_5 b_5
lw $s0, 0($sp)
lw $s1, 4($sp)
add $t0, $s0, $s1

# RET #Tmp5  
move $v0, $t0
jr $ra

# FUNC_END sum  
jr $ra

# FUNC_BEGIN main  
main :

# ADD a_10 1 0
lw $s0, 0($sp)
add $s0, $zero, 1

# ADD b_10 5 0
lw $s1, 4($sp)
add $s1, $zero, 5

# INIT_ARR_PTR c_11  
lw $s2, 8($sp)
add $s2, $sp, 12

# PREPARE_CALL sum  
sub $sp, $sp, 112

# PUSH_VAL a_10 0 
sw $s0, 0($sp)

# PUSH_VAL b_10 1 
sw $s1, 4($sp)

# CALL sum  
sw $ra, 12($sp)
sw $s0, 36($sp)
sw $s1, 40($sp)
sw $s2, 44($sp)
jal sum
lw $ra, 12($sp)
lw $s0, 36($sp)
lw $s1, 40($sp)
lw $s2, 44($sp)
add $sp, $sp, 112

# ADD #Tmp6 %RET 0
add $t0, $v0, 0

# PREPARE_CALL sum  
sub $sp, $sp, 112

# PUSH_VAL #Tmp6 0 
sw $t0, 0($sp)

# PUSH_VAL b_10 1 
sw $s1, 4($sp)

# CALL sum  
sw $ra, 12($sp)
sw $s0, 36($sp)
sw $s1, 40($sp)
sw $s2, 44($sp)
jal sum
lw $ra, 12($sp)
lw $s0, 36($sp)
lw $s1, 40($sp)
lw $s2, 44($sp)
add $sp, $sp, 112

# ADD #Tmp7 %RET 0
add $t1, $v0, 0

# PREPARE_CALL sum  
sub $sp, $sp, 112

# PUSH_VAL a_10 0 
sw $s0, 0($sp)

# PUSH_VAL #Tmp7 1 
sw $t1, 4($sp)

# CALL sum  
sw $ra, 12($sp)
sw $s0, 36($sp)
sw $s1, 40($sp)
sw $s2, 44($sp)
jal sum
lw $ra, 12($sp)
lw $s0, 36($sp)
lw $s1, 40($sp)
lw $s2, 44($sp)
add $sp, $sp, 112

# ADD #Tmp8 %RET 0
add $t2, $v0, 0

# ADD a_10 #Tmp8 0
add $s0, $t2, 0

# PRINT a:  str 
la $a0, str_0
li $v0, 4
syscall

# PRINT a_10 int 
move $a0, $s0
li $v0, 1
syscall

# RET 0  
add $v0, $zero, 0
li $v0, 10
syscall

# FUNC_END main  
