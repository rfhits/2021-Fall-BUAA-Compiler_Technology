.data
	str_0:	.asciiz "["
	str_1:	.asciiz ", "
	str_2:	.asciiz "]"
	str_3:	.asciiz "\n"
	str_4:	.asciiz "19373384"
	str_5:	.asciiz "Inner product: expect "
	str_6:	.asciiz ", output: "
	str_7:	.asciiz "SBC step 1: expect:"
	str_8:	.asciiz "output:"
	str_9:	.asciiz "SBC step 2 and EWM: expect:"
	str_10:	.asciiz "================================================================================"
	str_11:	.asciiz "                                     PASSED                                     "
.text

# ADD global_counter_1 0 0
lw $s0, 0($gp)
add $s0, $zero, 0

# FUNC_BEGIN innerProduct  
addi $sp, $sp, -324
j main
innerProduct :

# ADD i_3 0 0
lw $s0, 12($sp)
add $s0, $zero, 0

# ADD res_4 0 0
lw $s1, 16($sp)
add $s1, $zero, 0

# While_Begin_Label_0:
sw $s0, 12($sp)
sw $s1, 16($sp)
While_Begin_Label_0:

# LSS #Tmp0 i_3 length_2
lw $s0, 12($sp)
lw $s1, 8($sp)
slt $t0, $s0, $s1

# BEQ While_End_Label_0 #Tmp0 0
sw $s0, 12($sp)
sw $s1, 8($sp)
beq $t0, $0, While_End_Label_0

# ARR_LOAD #Tmp1 a_2 i_3
lw $s0, 0($sp)
lw $s1, 12($sp)
sll $a0, $s1, 2
add $a0, $a0, $s0
lw $t1, 0($a0)

# ARR_LOAD #Tmp2 b_2 i_3
lw $s2, 4($sp)
sll $a0, $s1, 2
add $a0, $a0, $s2
lw $t2, 0($a0)

# MUL #Tmp3 #Tmp1 #Tmp2
mul $t3, $t1, $t2

# ADD #Tmp4 res_4 #Tmp3
lw $s3, 16($sp)
add $t4, $s3, $t3

# ADD res_4 #Tmp4 0
add $s3, $t4, 0

# ADD #Tmp5 i_3 1
add $t5, $s1, 1

# ADD i_3 #Tmp5 0
add $s1, $t5, 0

# JUMP While_Begin_Label_0  
sw $s0, 0($sp)
sw $s1, 12($sp)
sw $s2, 4($sp)
sw $s3, 16($sp)
j While_Begin_Label_0

# While_End_Label_0:
While_End_Label_0:

# RET res_4  
lw $v0, 16($sp)
jr $ra

# FUNC_END innerProduct  
jr $ra

# FUNC_BEGIN scaleBiasCombination  
scaleBiasCombination :

# ADD i_12 0 0
lw $s0, 28($sp)
add $s0, $zero, 0

# While_Begin_Label_1:
sw $s0, 28($sp)
While_Begin_Label_1:

# LSS #Tmp6 i_12 length_11
lw $s0, 28($sp)
lw $s1, 24($sp)
slt $t0, $s0, $s1

# BEQ While_End_Label_1 #Tmp6 0
sw $s0, 28($sp)
sw $s1, 24($sp)
beq $t0, $0, While_End_Label_1

# ARR_LOAD #Tmp7 a_11 i_12
lw $s0, 4($sp)
lw $s1, 28($sp)
sll $a0, $s1, 2
add $a0, $a0, $s0
lw $t1, 0($a0)

# MUL #Tmp8 #Tmp7 s1_11
lw $s2, 8($sp)
mul $t2, $t1, $s2

# ARR_LOAD #Tmp9 b_11 i_12
lw $s3, 12($sp)
sll $a0, $s1, 2
add $a0, $a0, $s3
lw $t3, 0($a0)

# MUL #Tmp10 #Tmp9 s2_11
lw $s4, 16($sp)
mul $t4, $t3, $s4

# ADD #Tmp11 #Tmp8 #Tmp10
add $t5, $t2, $t4

# ADD #Tmp12 #Tmp11 bias_11
lw $s5, 20($sp)
add $t6, $t5, $s5

# ADD #Tmp13 #Tmp12 global_counter_1
lw $s6, 0($gp)
add $t7, $t6, $s6

# ARR_SAVE res_11 i_12 #Tmp13
lw $s7, 0($sp)
add $a0, $zero, $s1
sll $a0, $a0, 2
add $a0, $a0, $s7
sw $t7, 0($a0)

# ADD #Tmp14 i_12 1
add $t1, $s1, 1

# ADD i_12 #Tmp14 0
add $s1, $t1, 0

# JUMP While_Begin_Label_1  
sw $s0, 4($sp)
sw $s1, 28($sp)
sw $s2, 8($sp)
sw $s3, 12($sp)
sw $s4, 16($sp)
sw $s5, 20($sp)
sw $s6, 0($gp)
sw $s7, 0($sp)
j While_Begin_Label_1

# While_End_Label_1:
While_End_Label_1:

# ADD #Tmp15 global_counter_1 1
lw $s0, 0($gp)
add $t3, $s0, 1

# ADD global_counter_1 #Tmp15 0
add $s0, $t3, 0

# FUNC_END scaleBiasCombination  
jr $ra

# FUNC_BEGIN elementWiseMultiply  
elementWiseMultiply :

# ADD i_20 0 0
lw $s0, 16($sp)
add $s0, $zero, 0

# While_Begin_Label_2:
sw $s0, 16($sp)
While_Begin_Label_2:

# LSS #Tmp16 i_20 length_19
lw $s0, 16($sp)
lw $s1, 12($sp)
slt $t0, $s0, $s1

# BEQ While_End_Label_2 #Tmp16 0
sw $s0, 16($sp)
sw $s1, 12($sp)
beq $t0, $0, While_End_Label_2

# ARR_LOAD #Tmp17 a_19 i_20
lw $s0, 4($sp)
lw $s1, 16($sp)
sll $a0, $s1, 2
add $a0, $a0, $s0
lw $t1, 0($a0)

# ARR_LOAD #Tmp18 b_19 i_20
lw $s2, 8($sp)
sll $a0, $s1, 2
add $a0, $a0, $s2
lw $t2, 0($a0)

# MUL #Tmp19 #Tmp17 #Tmp18
mul $t3, $t1, $t2

# ARR_SAVE res_19 i_20 #Tmp19
lw $s3, 0($sp)
add $a0, $zero, $s1
sll $a0, $a0, 2
add $a0, $a0, $s3
sw $t3, 0($a0)

# ADD #Tmp20 i_20 1
add $t4, $s1, 1

# ADD i_20 #Tmp20 0
add $s1, $t4, 0

# JUMP While_Begin_Label_2  
sw $s0, 4($sp)
sw $s1, 16($sp)
sw $s2, 8($sp)
sw $s3, 0($sp)
j While_Begin_Label_2

# While_End_Label_2:
While_End_Label_2:

# FUNC_END elementWiseMultiply  
jr $ra

# FUNC_BEGIN printVector  
printVector :

# ARR_LOAD #Tmp21 vec_26 0
lw $s0, 0($sp)
lw $t0, 0($s0)

# PRINT [ str 
la $a0, str_0
li $v0, 4
syscall

# PRINT #Tmp21 int 
move $a0, $t0
li $v0, 1
syscall

# ADD i_28 1 0
lw $s1, 12($sp)
add $s1, $zero, 1

# While_Begin_Label_3:
sw $s0, 0($sp)
sw $s1, 12($sp)
While_Begin_Label_3:

# LSS #Tmp22 i_28 length_26
lw $s0, 12($sp)
lw $s1, 4($sp)
slt $t1, $s0, $s1

# BEQ While_End_Label_3 #Tmp22 0
sw $s0, 12($sp)
sw $s1, 4($sp)
beq $t1, $0, While_End_Label_3

# ARR_LOAD #Tmp23 vec_26 i_28
lw $s0, 0($sp)
lw $s1, 12($sp)
sll $a0, $s1, 2
add $a0, $a0, $s0
lw $t2, 0($a0)

# PRINT ,  str 
la $a0, str_1
li $v0, 4
syscall

# PRINT #Tmp23 int 
move $a0, $t2
li $v0, 1
syscall

# ADD #Tmp24 i_28 1
add $t3, $s1, 1

# ADD i_28 #Tmp24 0
add $s1, $t3, 0

# JUMP While_Begin_Label_3  
sw $s0, 0($sp)
sw $s1, 12($sp)
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

# INIT_ARR_PTR a_37  
lw $s1, 0($sp)
add $s1, $sp, 4

# ARR_SAVE a_37 0 1
add $a1, $0, 1
sw $a1, 0($s1)

# ARR_SAVE a_37 1 3
add $a1, $0, 3
sw $a1, 4($s1)

# ARR_SAVE a_37 2 5
add $a1, $0, 5
sw $a1, 8($s1)

# ARR_SAVE a_37 3 7
add $a1, $0, 7
sw $a1, 12($s1)

# ARR_SAVE a_37 4 9
add $a1, $0, 9
sw $a1, 16($s1)

# ARR_SAVE a_37 5 11
add $a1, $0, 11
sw $a1, 20($s1)

# ARR_SAVE a_37 6 2
add $a1, $0, 2
sw $a1, 24($s1)

# ARR_SAVE a_37 7 9
add $a1, $0, 9
sw $a1, 28($s1)

# ARR_SAVE a_37 8 7
add $a1, $0, 7
sw $a1, 32($s1)

# ARR_SAVE a_37 9 8
add $a1, $0, 8
sw $a1, 36($s1)

# INIT_ARR_PTR b_38  
lw $s2, 44($sp)
add $s2, $sp, 48

# ARR_SAVE b_38 0 2
add $a1, $0, 2
sw $a1, 0($s2)

# ARR_SAVE b_38 1 5
add $a1, $0, 5
sw $a1, 4($s2)

# ARR_SAVE b_38 2 4
add $a1, $0, 4
sw $a1, 8($s2)

# ARR_SAVE b_38 3 8
add $a1, $0, 8
sw $a1, 12($s2)

# ARR_SAVE b_38 4 9
add $a1, $0, 9
sw $a1, 16($s2)

# ARR_SAVE b_38 5 6
add $a1, $0, 6
sw $a1, 20($s2)

# ARR_SAVE b_38 6 3
add $a1, $0, 3
sw $a1, 24($s2)

# ARR_SAVE b_38 7 7
add $a1, $0, 7
sw $a1, 28($s2)

# ARR_SAVE b_38 8 1
add $a1, $0, 1
sw $a1, 32($s2)

# ARR_SAVE b_38 9 1
add $a1, $0, 1
sw $a1, 36($s2)

# INIT_ARR_PTR res1_39  
lw $s3, 88($sp)
add $s3, $sp, 92

# INIT_ARR_PTR res2_39  
lw $s4, 132($sp)
add $s4, $sp, 136

# PREPARE_CALL printVector  
sub $sp, $sp, 128

# PUSH_ARR a_37 0 
sw $s1, 0($sp)

# PUSH_VAL 10 1 
add $a0, $zero, 10
sw $a0, 4($sp)

# CALL printVector  
sw $s0, 0($gp)
sw $ra, 28($sp)
sw $s1, 56($sp)
sw $s2, 60($sp)
sw $s3, 64($sp)
sw $s4, 68($sp)
jal printVector
lw $ra, 28($sp)
lw $s1, 56($sp)
lw $s2, 60($sp)
lw $s3, 64($sp)
lw $s4, 68($sp)
add $sp, $sp, 128

# PREPARE_CALL printVector  
sub $sp, $sp, 128

# PUSH_ARR b_38 0 
sw $s2, 0($sp)

# PUSH_VAL 10 1 
add $a0, $zero, 10
sw $a0, 4($sp)

# CALL printVector  
sw $ra, 28($sp)
sw $s1, 56($sp)
sw $s2, 60($sp)
sw $s3, 64($sp)
sw $s4, 68($sp)
jal printVector
lw $ra, 28($sp)
lw $s1, 56($sp)
lw $s2, 60($sp)
lw $s3, 64($sp)
lw $s4, 68($sp)
add $sp, $sp, 128

# PREPARE_CALL innerProduct  
sub $sp, $sp, 144

# PUSH_ARR a_37 0 
sw $s1, 0($sp)

# PUSH_ARR b_38 1 
sw $s2, 4($sp)

# PUSH_VAL 10 2 
add $a0, $zero, 10
sw $a0, 8($sp)

# CALL innerProduct  
sw $ra, 44($sp)
sw $s1, 72($sp)
sw $s2, 76($sp)
sw $s3, 80($sp)
sw $s4, 84($sp)
jal innerProduct
lw $ra, 44($sp)
lw $s1, 72($sp)
lw $s2, 76($sp)
lw $s3, 80($sp)
lw $s4, 84($sp)
add $sp, $sp, 144

# ADD #Tmp25 %RET 0
add $t0, $v0, 0

# ADD result_40 #Tmp25 0
lw $s0, 176($sp)
add $s0, $t0, 0

# ADD INNER_PRODUCT_ANSWER_44 324 0
lw $s5, 184($sp)
add $s5, $zero, 324

# PRINT Inner product: expect  str 
la $a0, str_5
li $v0, 4
syscall

# PRINT INNER_PRODUCT_ANSWER_44 int 
move $a0, $s5
li $v0, 1
syscall

# PRINT , output:  str 
la $a0, str_6
li $v0, 4
syscall

# PRINT result_40 int 
move $a0, $s0
li $v0, 1
syscall

# PRINT \n str 
la $a0, str_3
li $v0, 4
syscall

# NEQ #Tmp26 INNER_PRODUCT_ANSWER_44 result_40
sne $t1, $s5, $s0

# BEQ Label_0 #Tmp26 0
sw $s0, 176($sp)
sw $s1, 0($sp)
sw $s2, 44($sp)
sw $s3, 88($sp)
sw $s4, 132($sp)
sw $s5, 184($sp)
beq $t1, $0, Label_0

# RET 1  
add $v0, $zero, 1
li $v0, 10
syscall

# Label_0:
Label_0:

# PREPARE_CALL scaleBiasCombination  
sub $sp, $sp, 172

# PUSH_ARR res1_39 0 
lw $s0, 260($sp)
sw $s0, 0($sp)

# PUSH_ARR a_37 1 
lw $s1, 172($sp)
sw $s1, 4($sp)

# PUSH_VAL 2 2 
add $a0, $zero, 2
sw $a0, 8($sp)

# PUSH_ARR b_38 3 
lw $s2, 216($sp)
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
sw $ra, 72($sp)
sw $s0, 96($sp)
sw $s1, 100($sp)
sw $s2, 104($sp)
sw $t1, 132($sp)
jal scaleBiasCombination
lw $ra, 72($sp)
lw $s0, 96($sp)
lw $s1, 100($sp)
lw $s2, 104($sp)
lw $t1, 132($sp)
add $sp, $sp, 172

# INIT_ARR_PTR SBC1_RESULT_49  
lw $s3, 192($sp)
add $s3, $sp, 196

# ARR_SAVE SBC1_RESULT_49 0 12
add $a1, $0, 12
sw $a1, 0($s3)

# ARR_SAVE SBC1_RESULT_49 1 25
add $a1, $0, 25
sw $a1, 4($s3)

# ARR_SAVE SBC1_RESULT_49 2 26
add $a1, $0, 26
sw $a1, 8($s3)

# ARR_SAVE SBC1_RESULT_49 3 42
add $a1, $0, 42
sw $a1, 12($s3)

# ARR_SAVE SBC1_RESULT_49 4 49
add $a1, $0, 49
sw $a1, 16($s3)

# ARR_SAVE SBC1_RESULT_49 5 44
add $a1, $0, 44
sw $a1, 20($s3)

# ARR_SAVE SBC1_RESULT_49 6 17
add $a1, $0, 17
sw $a1, 24($s3)

# ARR_SAVE SBC1_RESULT_49 7 43
add $a1, $0, 43
sw $a1, 28($s3)

# ARR_SAVE SBC1_RESULT_49 8 21
add $a1, $0, 21
sw $a1, 32($s3)

# ARR_SAVE SBC1_RESULT_49 9 23
add $a1, $0, 23
sw $a1, 36($s3)

# PRINT SBC step 1: expect: str 
la $a0, str_7
li $v0, 4
syscall

# PREPARE_CALL printVector  
sub $sp, $sp, 128

# PUSH_ARR SBC1_RESULT_49 0 
sw $s3, 0($sp)

# PUSH_VAL 10 1 
add $a0, $zero, 10
sw $a0, 4($sp)

# CALL printVector  
sw $ra, 28($sp)
sw $s0, 52($sp)
sw $s1, 56($sp)
sw $s2, 60($sp)
sw $s3, 64($sp)
sw $t1, 88($sp)
jal printVector
lw $ra, 28($sp)
lw $s0, 52($sp)
lw $s1, 56($sp)
lw $s2, 60($sp)
lw $s3, 64($sp)
lw $t1, 88($sp)
add $sp, $sp, 128

# PRINT output: str 
la $a0, str_8
li $v0, 4
syscall

# PREPARE_CALL printVector  
sub $sp, $sp, 128

# PUSH_ARR res1_39 0 
sw $s0, 0($sp)

# PUSH_VAL 10 1 
add $a0, $zero, 10
sw $a0, 4($sp)

# CALL printVector  
sw $ra, 28($sp)
sw $s0, 52($sp)
sw $s1, 56($sp)
sw $s2, 60($sp)
sw $s3, 64($sp)
sw $t1, 88($sp)
jal printVector
lw $ra, 28($sp)
lw $s0, 52($sp)
lw $s1, 56($sp)
lw $s2, 60($sp)
lw $s3, 64($sp)
lw $t1, 88($sp)
add $sp, $sp, 128

# ADD i_54 0 0
lw $s4, 236($sp)
add $s4, $zero, 0

# While_Begin_Label_4:
sw $s0, 88($sp)
sw $s1, 0($sp)
sw $s2, 44($sp)
sw $s3, 192($sp)
sw $s4, 236($sp)
While_Begin_Label_4:

# LSS #Tmp27 i_54 10
lw $s0, 236($sp)
slti $t2, $s0, 10

# BEQ While_End_Label_4 #Tmp27 0
sw $s0, 236($sp)
beq $t2, $0, While_End_Label_4

# ARR_LOAD #Tmp28 res1_39 i_54
lw $s0, 88($sp)
lw $s1, 236($sp)
sll $a0, $s1, 2
add $a0, $a0, $s0
lw $t3, 0($a0)

# ARR_LOAD #Tmp29 SBC1_RESULT_49 i_54
lw $s2, 192($sp)
sll $a0, $s1, 2
add $a0, $a0, $s2
lw $t4, 0($a0)

# NEQ #Tmp30 #Tmp28 #Tmp29
sne $t5, $t3, $t4

# BEQ Label_1 #Tmp30 0
sw $s0, 88($sp)
sw $s1, 236($sp)
sw $s2, 192($sp)
beq $t5, $0, Label_1

# RET 2  
add $v0, $zero, 2
li $v0, 10
syscall

# Label_1:
Label_1:

# ADD #Tmp31 i_54 1
lw $s0, 236($sp)
add $t6, $s0, 1

# ADD i_54 #Tmp31 0
add $s0, $t6, 0

# JUMP While_Begin_Label_4  
sw $s0, 236($sp)
j While_Begin_Label_4

# While_End_Label_4:
While_End_Label_4:

# PREPARE_CALL scaleBiasCombination  
sub $sp, $sp, 172

# PUSH_ARR res2_39 0 
lw $s0, 304($sp)
sw $s0, 0($sp)

# PUSH_ARR res1_39 1 
lw $s1, 260($sp)
sw $s1, 4($sp)

# PUSH_VAL 2 2 
add $a0, $zero, 2
sw $a0, 8($sp)

# PUSH_ARR b_38 3 
lw $s2, 216($sp)
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
sw $ra, 72($sp)
sw $s0, 96($sp)
sw $s1, 100($sp)
sw $s2, 104($sp)
sw $t1, 132($sp)
sw $t2, 136($sp)
sw $t5, 148($sp)
jal scaleBiasCombination
lw $ra, 72($sp)
lw $s0, 96($sp)
lw $s1, 100($sp)
lw $s2, 104($sp)
lw $t1, 132($sp)
lw $t2, 136($sp)
lw $t5, 148($sp)
add $sp, $sp, 172

# PREPARE_CALL elementWiseMultiply  
sub $sp, $sp, 140

# PUSH_ARR res1_39 0 
sw $s1, 0($sp)

# PUSH_ARR res2_39 1 
sw $s0, 4($sp)

# PUSH_ARR b_38 2 
sw $s2, 8($sp)

# PUSH_VAL 10 3 
add $a0, $zero, 10
sw $a0, 12($sp)

# CALL elementWiseMultiply  
sw $ra, 40($sp)
sw $s0, 64($sp)
sw $s1, 68($sp)
sw $s2, 72($sp)
sw $t1, 100($sp)
sw $t2, 104($sp)
sw $t5, 116($sp)
jal elementWiseMultiply
lw $ra, 40($sp)
lw $s0, 64($sp)
lw $s1, 68($sp)
lw $s2, 72($sp)
lw $t1, 100($sp)
lw $t2, 104($sp)
lw $t5, 116($sp)
add $sp, $sp, 140

# INIT_ARR_PTR FINAL_RESULT_62  
lw $s3, 260($sp)
add $s3, $sp, 264

# ARR_SAVE FINAL_RESULT_62 0 70
add $a1, $0, 70
sw $a1, 0($s3)

# ARR_SAVE FINAL_RESULT_62 1 350
add $a1, $0, 350
sw $a1, 4($s3)

# ARR_SAVE FINAL_RESULT_62 2 276
add $a1, $0, 276
sw $a1, 8($s3)

# ARR_SAVE FINAL_RESULT_62 3 904
add $a1, $0, 904
sw $a1, 12($s3)

# ARR_SAVE FINAL_RESULT_62 4 1170
add $a1, $0, 1170
sw $a1, 16($s3)

# ARR_SAVE FINAL_RESULT_62 5 666
add $a1, $0, 666
sw $a1, 20($s3)

# ARR_SAVE FINAL_RESULT_62 6 144
add $a1, $0, 144
sw $a1, 24($s3)

# ARR_SAVE FINAL_RESULT_62 7 784
add $a1, $0, 784
sw $a1, 28($s3)

# ARR_SAVE FINAL_RESULT_62 8 50
add $a1, $0, 50
sw $a1, 32($s3)

# ARR_SAVE FINAL_RESULT_62 9 54
add $a1, $0, 54
sw $a1, 36($s3)

# PRINT SBC step 2 and EWM: expect: str 
la $a0, str_9
li $v0, 4
syscall

# PREPARE_CALL printVector  
sub $sp, $sp, 128

# PUSH_ARR FINAL_RESULT_62 0 
sw $s3, 0($sp)

# PUSH_VAL 10 1 
add $a0, $zero, 10
sw $a0, 4($sp)

# CALL printVector  
sw $ra, 28($sp)
sw $s0, 52($sp)
sw $s1, 56($sp)
sw $s2, 60($sp)
sw $s3, 64($sp)
sw $t1, 88($sp)
sw $t2, 92($sp)
sw $t5, 104($sp)
jal printVector
lw $ra, 28($sp)
lw $s0, 52($sp)
lw $s1, 56($sp)
lw $s2, 60($sp)
lw $s3, 64($sp)
lw $t1, 88($sp)
lw $t2, 92($sp)
lw $t5, 104($sp)
add $sp, $sp, 128

# PRINT output: str 
la $a0, str_8
li $v0, 4
syscall

# PREPARE_CALL printVector  
sub $sp, $sp, 128

# PUSH_ARR res1_39 0 
sw $s1, 0($sp)

# PUSH_VAL 10 1 
add $a0, $zero, 10
sw $a0, 4($sp)

# CALL printVector  
sw $ra, 28($sp)
sw $s0, 52($sp)
sw $s1, 56($sp)
sw $s2, 60($sp)
sw $s3, 64($sp)
sw $t1, 88($sp)
sw $t2, 92($sp)
sw $t5, 104($sp)
jal printVector
lw $ra, 28($sp)
lw $s0, 52($sp)
lw $s1, 56($sp)
lw $s2, 60($sp)
lw $s3, 64($sp)
lw $t1, 88($sp)
lw $t2, 92($sp)
lw $t5, 104($sp)
add $sp, $sp, 128

# ADD i_54 0 0
lw $s4, 236($sp)
add $s4, $zero, 0

# While_Begin_Label_5:
sw $s0, 132($sp)
sw $s1, 88($sp)
sw $s2, 44($sp)
sw $s3, 260($sp)
sw $s4, 236($sp)
While_Begin_Label_5:

# LSS #Tmp32 i_54 10
lw $s0, 236($sp)
slti $t7, $s0, 10

# BEQ While_End_Label_5 #Tmp32 0
sw $s0, 236($sp)
beq $t7, $0, While_End_Label_5

# ARR_LOAD #Tmp33 res1_39 i_54
lw $s0, 88($sp)
lw $s1, 236($sp)
sll $a0, $s1, 2
add $a0, $a0, $s0
lw $t0, 0($a0)

# ARR_LOAD #Tmp34 FINAL_RESULT_62 i_54
lw $s2, 260($sp)
sll $a0, $s1, 2
add $a0, $a0, $s2
lw $t3, 0($a0)

# NEQ #Tmp35 #Tmp33 #Tmp34
sne $t4, $t0, $t3

# BEQ Label_2 #Tmp35 0
sw $s0, 88($sp)
sw $s1, 236($sp)
sw $s2, 260($sp)
beq $t4, $0, Label_2

# RET 3  
add $v0, $zero, 3
li $v0, 10
syscall

# Label_2:
Label_2:

# ADD #Tmp36 i_54 1
lw $s0, 236($sp)
add $t6, $s0, 1

# ADD i_54 #Tmp36 0
add $s0, $t6, 0

# JUMP While_Begin_Label_5  
sw $s0, 236($sp)
j While_Begin_Label_5

# While_End_Label_5:
While_End_Label_5:

# PRINT \n str 
la $a0, str_3
li $v0, 4
syscall

# PRINT \n str 
la $a0, str_3
li $v0, 4
syscall

# PRINT ================================================================================ str 
la $a0, str_10
li $v0, 4
syscall

# PRINT \n str 
la $a0, str_3
li $v0, 4
syscall

# PRINT                                      PASSED                                      str 
la $a0, str_11
li $v0, 4
syscall

# PRINT \n str 
la $a0, str_3
li $v0, 4
syscall

# PRINT ================================================================================ str 
la $a0, str_10
li $v0, 4
syscall

# PRINT \n str 
la $a0, str_3
li $v0, 4
syscall

# RET 0  
add $v0, $zero, 0
li $v0, 10
syscall

# FUNC_END main  
