.data
	str_0:	.asciiz "\n"
.text

# FUNC_BEGIN f2  
addi $sp, $sp, -20
j main
f2 :

# ARR_SAVE x_2 0 100
lw $s0, 0($sp)
add $a1, $zero, 100
sw $a1, 0($s0)

# RET 0  
add $v0, $zero, 0
jr $ra

# FUNC_END f2  
jr $ra

# FUNC_BEGIN main  
main :

# ARR_SAVE a_8 0 0
add $s0, $sp, 0
add $a1, $zero, 0
sw $a1, 0($s0)

# PREPARE_CALL f2  
sub $sp, $sp, 104

# ARR_LOAD #Tmp0 a_8 0
lw $t0, 0($s0)

# ARR_SAVE @Arr0 0 #Tmp0
add $s1, $sp, 108
sw $t0, 0($s1)

# PUSH_ARR @Arr0 0 
add $a0, $sp, 108
sw $a0, 0($sp)

# CALL f2  
sw $ra, 4($sp)
sw $s0, 28($sp)
sw $s1, 32($sp)
sw $t0, 60($sp)
jal f2
lw $ra, 4($sp)
lw $s0, 28($sp)
lw $s1, 32($sp)
lw $t0, 60($sp)
add $sp, $sp, 104

# ADD #Tmp1 %RET 0
add $t1, $v0, 0

# ARR_LOAD #Tmp2 a_8 0
lw $t2, 0($s0)

# PRINT #Tmp2 int 
move $a0, $t2
li $v0, 1
syscall

# PRINT \n str 
la $a0, str_0
li $v0, 4
syscall

# RET 0  
add $v0, $zero, 0
li $v0, 10
syscall

# FUNC_END main  
