.data
	str_0:	.asciiz "["
	str_1:	.asciiz ", "
	str_2:	.asciiz "]"
	str_3:	.asciiz "\n"
	str_4:	.asciiz "19373384"
	str_5:	.asciiz "Inner product: "
	str_6:	.asciiz "first scale: "
	str_7:	.asciiz "second scale: "
	str_8:	.asciiz "wise multi: "
.text

# ADD global_counter_2 0 0
lw $s0, 0($gp)
add $s0, $zero, 0

# FUNC_BEGIN innerProduct  
addi $sp, $sp, -184
j main
innerProduct :

# ADD i_4 0 0
lw $s0, 12($sp)
add $s0, $zero, 0

# ADD res_5 0 0
lw $s1, 16($sp)
add $s1, $zero, 0

# While_Begin_Label_0:
sw $s0, 12($sp)
sw $s1, 16($sp)
While_Begin_Label_0:

# LSS #Tmp2 i_4 length_3
lw $s0, 12($sp)
lw $s1, 8($sp)
slt $t0, $s0, $s1

# ADD #Tmp1 #Tmp2 0
add $t1, $t0, 0

# LAnd_End_Label_0:
sw $s0, 12($sp)
sw $s1, 8($sp)
sw $t1, 24($sp)
LAnd_End_Label_0:

# ADD #Tmp0 #Tmp1 0
lw $t2, 24($sp)
add $t1, $t2, 0

# Cond_End_Label_0:
sw $t1, 20($sp)
Cond_End_Label_0:

# BEQ While_End_Label_0 #Tmp0 0
lw $t1, 20($sp)
sw $t1, 20($sp)
beq $t1, $0, While_End_Label_0

# ARR_LOAD #Tmp3 a_3 i_4
lw $s0, 0($sp)
lw $s1, 12($sp)
sll $a0, $s1, 2
add $a0, $a0, $s0
lw $t1, 0($a0)

# ARR_LOAD #Tmp4 b_3 i_4
lw $s2, 4($sp)
sll $a0, $s1, 2
add $a0, $a0, $s2
lw $t3, 0($a0)

# MUL #Tmp5 #Tmp3 #Tmp4
mul $t4, $t1, $t3

# ADD #Tmp6 res_5 #Tmp5
lw $s3, 16($sp)
add $t5, $s3, $t4

# ADD res_5 #Tmp6 0
add $s3, $t5, 0

# ADD #Tmp7 i_4 1
add $t6, $s1, 1

# ADD i_4 #Tmp7 0
add $s1, $t6, 0

# JUMP While_Begin_Label_0  
sw $s0, 0($sp)
sw $s1, 12($sp)
sw $s2, 4($sp)
sw $s3, 16($sp)
j While_Begin_Label_0

# While_End_Label_0:
While_End_Label_0:

# RET res_5  
lw $v0, 16($sp)
jr $ra

# FUNC_END innerProduct  
jr $ra

# FUNC_BEGIN scaleBiasCombination  
scaleBiasCombination :

# ADD i_13 0 0
lw $s0, 28($sp)
add $s0, $zero, 0

# While_Begin_Label_1:
sw $s0, 28($sp)
While_Begin_Label_1:

# LSS #Tmp10 i_13 length_12
lw $s0, 28($sp)
lw $s1, 24($sp)
slt $t0, $s0, $s1

# ADD #Tmp9 #Tmp10 0
add $t1, $t0, 0

# LAnd_End_Label_1:
sw $s0, 28($sp)
sw $s1, 24($sp)
sw $t1, 36($sp)
LAnd_End_Label_1:

# ADD #Tmp8 #Tmp9 0
lw $t2, 36($sp)
add $t1, $t2, 0

# Cond_End_Label_1:
sw $t1, 32($sp)
Cond_End_Label_1:

# BEQ While_End_Label_1 #Tmp8 0
lw $t1, 32($sp)
sw $t1, 32($sp)
beq $t1, $0, While_End_Label_1

# ARR_LOAD #Tmp11 a_12 i_13
lw $s0, 4($sp)
lw $s1, 28($sp)
sll $a0, $s1, 2
add $a0, $a0, $s0
lw $t1, 0($a0)

# MUL #Tmp12 #Tmp11 s1_12
lw $s2, 8($sp)
mul $t3, $t1, $s2

# ARR_LOAD #Tmp13 b_12 i_13
lw $s3, 12($sp)
sll $a0, $s1, 2
add $a0, $a0, $s3
lw $t4, 0($a0)

# MUL #Tmp14 #Tmp13 s2_12
lw $s4, 16($sp)
mul $t5, $t4, $s4

# ADD #Tmp15 #Tmp12 #Tmp14
add $t6, $t3, $t5

# ADD #Tmp16 #Tmp15 bias_12
lw $s5, 20($sp)
add $t7, $t6, $s5

# ADD #Tmp17 #Tmp16 global_counter_2
lw $s6, 0($gp)
add $t0, $t7, $s6

# ARR_SAVE res_12 i_13 #Tmp17
lw $s7, 0($sp)
add $a0, $zero, $s1
sll $a0, $a0, 2
add $a0, $a0, $s7
sw $t0, 0($a0)

# ADD #Tmp18 i_13 1
add $t2, $s1, 1

# ADD i_13 #Tmp18 0
add $s1, $t2, 0

# JUMP While_Begin_Label_1  
sw $s0, 4($sp)
sw $s1, 28($sp)
sw $s2, 8($sp)
sw $s3, 12($sp)
sw $s4, 16($sp)
sw $s5, 20($sp)
sw $s6, 0($gp)
sw $s7, 0($sp)
sw $t0, 68($sp)
j While_Begin_Label_1

# While_End_Label_1:
While_End_Label_1:

# ADD #Tmp19 global_counter_2 1
lw $s0, 0($gp)
add $t1, $s0, 1

# ADD global_counter_2 #Tmp19 0
add $s0, $t1, 0

# FUNC_END scaleBiasCombination  
sw $s0, 0($gp)
jr $ra

# FUNC_BEGIN elementWiseMultiply  
elementWiseMultiply :

# ADD i_21 0 0
lw $s0, 16($sp)
add $s0, $zero, 0

# While_Begin_Label_2:
sw $s0, 16($sp)
While_Begin_Label_2:

# LSS #Tmp22 i_21 length_20
lw $s0, 16($sp)
lw $s1, 12($sp)
slt $t0, $s0, $s1

# ADD #Tmp21 #Tmp22 0
add $t1, $t0, 0

# LAnd_End_Label_2:
sw $s0, 16($sp)
sw $s1, 12($sp)
sw $t1, 24($sp)
LAnd_End_Label_2:

# ADD #Tmp20 #Tmp21 0
lw $t2, 24($sp)
add $t1, $t2, 0

# Cond_End_Label_2:
sw $t1, 20($sp)
Cond_End_Label_2:

# BEQ While_End_Label_2 #Tmp20 0
lw $t1, 20($sp)
sw $t1, 20($sp)
beq $t1, $0, While_End_Label_2

# ARR_LOAD #Tmp23 a_20 i_21
lw $s0, 4($sp)
lw $s1, 16($sp)
sll $a0, $s1, 2
add $a0, $a0, $s0
lw $t1, 0($a0)

# ARR_LOAD #Tmp24 b_20 i_21
lw $s2, 8($sp)
sll $a0, $s1, 2
add $a0, $a0, $s2
lw $t3, 0($a0)

# MUL #Tmp25 #Tmp23 #Tmp24
mul $t4, $t1, $t3

# ARR_SAVE res_20 i_21 #Tmp25
lw $s3, 0($sp)
add $a0, $zero, $s1
sll $a0, $a0, 2
add $a0, $a0, $s3
sw $t4, 0($a0)

# ADD #Tmp26 i_21 1
add $t5, $s1, 1

# ADD i_21 #Tmp26 0
add $s1, $t5, 0

# JUMP While_Begin_Label_2  
sw $s0, 4($sp)
sw $s1, 16($sp)
sw $s2, 8($sp)
sw $s3, 0($sp)
sw $t4, 40($sp)
j While_Begin_Label_2

# While_End_Label_2:
While_End_Label_2:

# FUNC_END elementWiseMultiply  
jr $ra

# FUNC_BEGIN printVector  
printVector :

# ARR_LOAD #Tmp27 vec_27 0
lw $s0, 0($sp)
lw $t0, 0($s0)

# PRINT [ str 
la $a0, str_0
li $v0, 4
syscall

# PRINT #Tmp27 int 
move $a0, $t0
li $v0, 1
syscall

# ADD i_29 1 0
lw $s1, 12($sp)
add $s1, $zero, 1

# While_Begin_Label_3:
sw $s0, 0($sp)
sw $s1, 12($sp)
sw $t0, 8($sp)
While_Begin_Label_3:

# LSS #Tmp30 i_29 length_27
lw $s0, 12($sp)
lw $s1, 4($sp)
slt $t0, $s0, $s1

# ADD #Tmp29 #Tmp30 0
add $t1, $t0, 0

# LAnd_End_Label_3:
sw $s0, 12($sp)
sw $s1, 4($sp)
sw $t1, 20($sp)
LAnd_End_Label_3:

# ADD #Tmp28 #Tmp29 0
lw $t2, 20($sp)
add $t1, $t2, 0

# Cond_End_Label_3:
sw $t1, 16($sp)
Cond_End_Label_3:

# BEQ While_End_Label_3 #Tmp28 0
lw $t1, 16($sp)
sw $t1, 16($sp)
beq $t1, $0, While_End_Label_3

# ARR_LOAD #Tmp31 vec_27 i_29
lw $s0, 0($sp)
lw $s1, 12($sp)
sll $a0, $s1, 2
add $a0, $a0, $s0
lw $t1, 0($a0)

# PRINT ,  str 
la $a0, str_1
li $v0, 4
syscall

# PRINT #Tmp31 int 
move $a0, $t1
li $v0, 1
syscall

# ADD #Tmp32 i_29 1
add $t3, $s1, 1

# ADD i_29 #Tmp32 0
add $s1, $t3, 0

# JUMP While_Begin_Label_3  
sw $s0, 0($sp)
sw $s1, 12($sp)
sw $t1, 28($sp)
j While_Begin_Label_3

# While_End_Label_3:
While_End_Label_3:

# PRINT ] str 
la $a0, str_2
li $v0, 4
syscall

# PRINT \n str 
la $a0, str_3
li $v0, 4
syscall

# FUNC_END printVector  
jr $ra

# FUNC_BEGIN main  
main :

# PRINT 19373384 str 
la $a0, str_4
li $v0, 4
syscall

# PRINT \n str 
la $a0, str_3
li $v0, 4
syscall

# INIT_ARR_PTR a_38  
lw $s1, 0($sp)
add $s1, $sp, 4

# ARR_SAVE a_38 0 1
add $a1, $0, 1
sw $a1, 0($s1)

# ARR_SAVE a_38 1 3
add $a1, $0, 3
sw $a1, 4($s1)

# ARR_SAVE a_38 2 5
add $a1, $0, 5
sw $a1, 8($s1)

# ARR_SAVE a_38 3 7
add $a1, $0, 7
sw $a1, 12($s1)

# ARR_SAVE a_38 4 9
add $a1, $0, 9
sw $a1, 16($s1)

# ARR_SAVE a_38 5 11
add $a1, $0, 11
sw $a1, 20($s1)

# ARR_SAVE a_38 6 2
add $a1, $0, 2
sw $a1, 24($s1)

# ARR_SAVE a_38 7 9
add $a1, $0, 9
sw $a1, 28($s1)

# ARR_SAVE a_38 8 7
add $a1, $0, 7
sw $a1, 32($s1)

# ARR_SAVE a_38 9 8
add $a1, $0, 8
sw $a1, 36($s1)

# INIT_ARR_PTR b_39  
lw $s2, 44($sp)
add $s2, $sp, 48

# ARR_SAVE b_39 0 2
add $a1, $0, 2
sw $a1, 0($s2)

# ARR_SAVE b_39 1 5
add $a1, $0, 5
sw $a1, 4($s2)

# ARR_SAVE b_39 2 4
add $a1, $0, 4
sw $a1, 8($s2)

# ARR_SAVE b_39 3 8
add $a1, $0, 8
sw $a1, 12($s2)

# ARR_SAVE b_39 4 9
add $a1, $0, 9
sw $a1, 16($s2)

# ARR_SAVE b_39 5 6
add $a1, $0, 6
sw $a1, 20($s2)

# ARR_SAVE b_39 6 3
add $a1, $0, 3
sw $a1, 24($s2)

# ARR_SAVE b_39 7 7
add $a1, $0, 7
sw $a1, 28($s2)

# ARR_SAVE b_39 8 1
add $a1, $0, 1
sw $a1, 32($s2)

# ARR_SAVE b_39 9 1
add $a1, $0, 1
sw $a1, 36($s2)

# INIT_ARR_PTR res1_40  
lw $s3, 88($sp)
add $s3, $sp, 92

# INIT_ARR_PTR res2_40  
lw $s4, 132($sp)
add $s4, $sp, 136

# PREPARE_CALL printVector  
sub $sp, $sp, 136

# PUSH_ARR a_38 0 
sw $s1, 0($sp)

# PUSH_VAL 10 1 
add $a0, $zero, 10
sw $a0, 4($sp)

# CALL printVector  
sw $s0, 0($gp)
sw $ra, 36($sp)
sw $s1, 64($sp)
sw $s2, 68($sp)
sw $s3, 72($sp)
sw $s4, 76($sp)
jal printVector
lw $ra, 36($sp)
lw $s1, 64($sp)
lw $s2, 68($sp)
lw $s3, 72($sp)
lw $s4, 76($sp)
add $sp, $sp, 136

# PREPARE_CALL printVector  
sub $sp, $sp, 136

# PUSH_ARR b_39 0 
sw $s2, 0($sp)

# PUSH_VAL 10 1 
add $a0, $zero, 10
sw $a0, 4($sp)

# CALL printVector  
sw $ra, 36($sp)
sw $s1, 64($sp)
sw $s2, 68($sp)
sw $s3, 72($sp)
sw $s4, 76($sp)
jal printVector
lw $ra, 36($sp)
lw $s1, 64($sp)
lw $s2, 68($sp)
lw $s3, 72($sp)
lw $s4, 76($sp)
add $sp, $sp, 136

# PREPARE_CALL innerProduct  
sub $sp, $sp, 152

# PUSH_ARR a_38 0 
sw $s1, 0($sp)

# PUSH_ARR b_39 1 
sw $s2, 4($sp)

# PUSH_VAL 10 2 
add $a0, $zero, 10
sw $a0, 8($sp)

# CALL innerProduct  
sw $ra, 52($sp)
sw $s1, 80($sp)
sw $s2, 84($sp)
sw $s3, 88($sp)
sw $s4, 92($sp)
jal innerProduct
lw $ra, 52($sp)
lw $s1, 80($sp)
lw $s2, 84($sp)
lw $s3, 88($sp)
lw $s4, 92($sp)
add $sp, $sp, 152

# ADD #Tmp33 %RET 0
add $t0, $v0, 0

# ADD result_41 #Tmp33 0
lw $s0, 176($sp)
add $s0, $t0, 0

# PRINT Inner product:  str 
la $a0, str_5
li $v0, 4
syscall

# PRINT result_41 int 
move $a0, $s0
li $v0, 1
syscall

# PRINT \n str 
la $a0, str_3
li $v0, 4
syscall

# PREPARE_CALL scaleBiasCombination  
sub $sp, $sp, 180

# PUSH_ARR res1_40 0 
sw $s3, 0($sp)

# PUSH_ARR a_38 1 
sw $s1, 4($sp)

# PUSH_VAL 2 2 
add $a0, $zero, 2
sw $a0, 8($sp)

# PUSH_ARR b_39 3 
sw $s2, 12($sp)

# PUSH_VAL 3 4 
add $a0, $zero, 3
sw $a0, 16($sp)

# PUSH_VAL 4 5 
add $a0, $zero, 4
sw $a0, 20($sp)

# PUSH_VAL 10 6 
add $a0, $zero, 10
sw $a0, 24($sp)

# CALL scaleBiasCombination  
sw $ra, 80($sp)
sw $s0, 104($sp)
sw $s1, 108($sp)
sw $s2, 112($sp)
sw $s3, 116($sp)
sw $s4, 120($sp)
jal scaleBiasCombination
lw $ra, 80($sp)
lw $s0, 104($sp)
lw $s1, 108($sp)
lw $s2, 112($sp)
lw $s3, 116($sp)
lw $s4, 120($sp)
add $sp, $sp, 180

# PRINT first scale:  str 
la $a0, str_6
li $v0, 4
syscall

# PRINT \n str 
la $a0, str_3
li $v0, 4
syscall

# PREPARE_CALL printVector  
sub $sp, $sp, 136

# PUSH_ARR res1_40 0 
sw $s3, 0($sp)

# PUSH_VAL 10 1 
add $a0, $zero, 10
sw $a0, 4($sp)

# CALL printVector  
sw $ra, 36($sp)
sw $s0, 60($sp)
sw $s1, 64($sp)
sw $s2, 68($sp)
sw $s3, 72($sp)
sw $s4, 76($sp)
jal printVector
lw $ra, 36($sp)
lw $s0, 60($sp)
lw $s1, 64($sp)
lw $s2, 68($sp)
lw $s3, 72($sp)
lw $s4, 76($sp)
add $sp, $sp, 136

# PREPARE_CALL scaleBiasCombination  
sub $sp, $sp, 180

# PUSH_ARR res2_40 0 
sw $s4, 0($sp)

# PUSH_ARR res1_40 1 
sw $s3, 4($sp)

# PUSH_VAL 2 2 
add $a0, $zero, 2
sw $a0, 8($sp)

# PUSH_ARR b_39 3 
sw $s2, 12($sp)

# PUSH_VAL 3 4 
add $a0, $zero, 3
sw $a0, 16($sp)

# PUSH_VAL 4 5 
add $a0, $zero, 4
sw $a0, 20($sp)

# PUSH_VAL 10 6 
add $a0, $zero, 10
sw $a0, 24($sp)

# CALL scaleBiasCombination  
sw $ra, 80($sp)
sw $s0, 104($sp)
sw $s1, 108($sp)
sw $s2, 112($sp)
sw $s3, 116($sp)
sw $s4, 120($sp)
jal scaleBiasCombination
lw $ra, 80($sp)
lw $s0, 104($sp)
lw $s1, 108($sp)
lw $s2, 112($sp)
lw $s3, 116($sp)
lw $s4, 120($sp)
add $sp, $sp, 180

# PRINT second scale:  str 
la $a0, str_7
li $v0, 4
syscall

# PRINT \n str 
la $a0, str_3
li $v0, 4
syscall

# PREPARE_CALL printVector  
sub $sp, $sp, 136

# PUSH_ARR res2_40 0 
sw $s4, 0($sp)

# PUSH_VAL 10 1 
add $a0, $zero, 10
sw $a0, 4($sp)

# CALL printVector  
sw $ra, 36($sp)
sw $s0, 60($sp)
sw $s1, 64($sp)
sw $s2, 68($sp)
sw $s3, 72($sp)
sw $s4, 76($sp)
jal printVector
lw $ra, 36($sp)
lw $s0, 60($sp)
lw $s1, 64($sp)
lw $s2, 68($sp)
lw $s3, 72($sp)
lw $s4, 76($sp)
add $sp, $sp, 136

# PREPARE_CALL elementWiseMultiply  
sub $sp, $sp, 148

# PUSH_ARR res1_40 0 
sw $s3, 0($sp)

# PUSH_ARR res2_40 1 
sw $s4, 4($sp)

# PUSH_ARR b_39 2 
sw $s2, 8($sp)

# PUSH_VAL 10 3 
add $a0, $zero, 10
sw $a0, 12($sp)

# CALL elementWiseMultiply  
sw $ra, 48($sp)
sw $s0, 72($sp)
sw $s1, 76($sp)
sw $s2, 80($sp)
sw $s3, 84($sp)
sw $s4, 88($sp)
jal elementWiseMultiply
lw $ra, 48($sp)
lw $s0, 72($sp)
lw $s1, 76($sp)
lw $s2, 80($sp)
lw $s3, 84($sp)
lw $s4, 88($sp)
add $sp, $sp, 148

# PRINT wise multi:  str 
la $a0, str_8
li $v0, 4
syscall

# PRINT \n str 
la $a0, str_3
li $v0, 4
syscall

# PREPARE_CALL printVector  
sub $sp, $sp, 136

# PUSH_ARR res1_40 0 
sw $s3, 0($sp)

# PUSH_VAL 10 1 
add $a0, $zero, 10
sw $a0, 4($sp)

# CALL printVector  
sw $ra, 36($sp)
sw $s0, 60($sp)
sw $s1, 64($sp)
sw $s2, 68($sp)
sw $s3, 72($sp)
sw $s4, 76($sp)
jal printVector
lw $ra, 36($sp)
lw $s0, 60($sp)
lw $s1, 64($sp)
lw $s2, 68($sp)
lw $s3, 72($sp)
lw $s4, 76($sp)
add $sp, $sp, 136

# RET 0  
add $v0, $zero, 0
li $v0, 10
syscall

# FUNC_END main  
