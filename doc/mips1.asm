.data
	str_0:	.asciiz "aa"
	str_1:	.asciiz "\n"
.text
sub $gp, $gp, 48

# ADD a_1 1 0
lw $s0, 0($gp)
add $s0, $zero, 1

# ARR_SAVE arr_2 0 4
add $a1, $zero, 4
sw $a1, 4($gp)

# ARR_SAVE arr_2 1 5
add $a1, $zero, 5
sw $a1, 8($gp)

# ARR_SAVE arr_2 2 2
add $a1, $zero, 2
sw $a1, 12($gp)

# ARR_SAVE arr_2 3 3
add $a1, $zero, 3
sw $a1, 16($gp)

# ARR_SAVE arr_2 4 0
add $a1, $zero, 0
sw $a1, 20($gp)

# ARR_SAVE arr_2 5 1
add $a1, $zero, 1
sw $a1, 24($gp)

# ADD #Tmp0 a_1 1
add $t0, $s0, 1

# ADD #Tmp1 #Tmp0 a_1
add $t1, $t0, $s0

# ADD #Tmp2 #Tmp1 1
add $t2, $t1, 1

# ADD #Tmp3 #Tmp2 a_1
add $t3, $t2, $s0

# ADD b_3 #Tmp3 0
lw $s1, 28($gp)
add $s1, $t3, 0

# FUNC_BEGIN f1  
addi $sp, $sp, -12
j main
f1 :

# RET 0  
add $v0, $zero, 0
jr $ra

# FUNC_END f1  

# FUNC_BEGIN f2  
f2 :

# RET 0  
add $v0, $zero, 0
jr $ra

# FUNC_END f2  

# FUNC_BEGIN main  
main :

# PREPARE_CALL f2  
sub $sp, $sp, -104

# PREPARE_CALL f1  
sub $sp, $sp, -104

# PUSH_VAL a_1 0 
lw $a0, 0($gp)
sw $a0, 0($sp)

# CALL f1  
sw $ra, 4($sp)
jal f1
lw $ra, 4($sp)
add $sp, $sp, 104

# ADD #Tmp4 %RET 0
add $t4, $v0, 0

# PUSH_VAL #Tmp4 0 
sw $t4, 0($sp)

# CALL f2  
sw $ra, 4($sp)
jal f2
lw $ra, 4($sp)
add $sp, $sp, 104

# ADD #Tmp5 %RET 0
add $t5, $v0, 0

# GETINT n_16  
li $v0, 5
syscall
sw $v0, 8($sp)

# PRINT aa str 
la $a0, str_0
li $v0, 4
syscall

# PRINT \n str 
la $a0, str_1
li $v0, 4
syscall

# PRINT n_16 int 
lw $a0, 8($sp)
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
