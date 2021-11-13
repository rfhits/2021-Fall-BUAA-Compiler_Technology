.data
	str_0:	.asciiz "19182650"
	str_1:	.asciiz "\n"
.text

# FUNC_BEGIN aa  
addi $sp, $sp, -216
j main
aa :

# MUL #Tmp1 0 3
add $t0, $zero, 0

# ADD #Tmp1 #Tmp1 0
add $t0, $t0, 0

# ARR_LOAD #Tmp0 b_1 #Tmp1
lw $s0, 0($sp)
sll $a0, $t0, 2
add $a0, $a0, $s0
lw $t1, 0($a0)

# MUL #Tmp3 0 3
add $t2, $zero, 0

# ADD #Tmp3 #Tmp3 1
add $t2, $t2, 1

# ARR_LOAD #Tmp2 b_1 #Tmp3
sll $a0, $t2, 2
add $a0, $a0, $s0
lw $t3, 0($a0)

# ADD #Tmp4 #Tmp0 #Tmp2
add $t4, $t1, $t3

# MUL #Tmp6 0 3
add $t5, $zero, 0

# ADD #Tmp6 #Tmp6 2
add $t5, $t5, 2

# ARR_LOAD #Tmp5 b_1 #Tmp6
sll $a0, $t5, 2
add $a0, $a0, $s0
lw $t6, 0($a0)

# ADD #Tmp7 #Tmp4 #Tmp5
add $t7, $t4, $t6

# MUL #Tmp9 1 3
add $t1, $zero, 3

# ADD #Tmp9 #Tmp9 0
add $t1, $t1, 0

# ARR_LOAD #Tmp8 b_1 #Tmp9
sll $a0, $t1, 2
add $a0, $a0, $s0
lw $t3, 0($a0)

# ADD #Tmp10 #Tmp7 #Tmp8
add $t4, $t7, $t3

# MUL #Tmp12 1 3
add $t6, $zero, 3

# ADD #Tmp12 #Tmp12 1
add $t6, $t6, 1

# ARR_LOAD #Tmp11 b_1 #Tmp12
sll $a0, $t6, 2
add $a0, $a0, $s0
lw $t7, 0($a0)

# ADD #Tmp13 #Tmp10 #Tmp11
add $t3, $t4, $t7

# MUL #Tmp15 1 3
add $t4, $zero, 3

# ADD #Tmp15 #Tmp15 2
add $t4, $t4, 2

# ARR_LOAD #Tmp14 b_1 #Tmp15
sll $a0, $t4, 2
add $a0, $a0, $s0
lw $t7, 0($a0)

# ADD #Tmp16 #Tmp13 #Tmp14
sw $t0, 12($sp)
add $t0, $t3, $t7

# ADD w_3 #Tmp16 0
lw $s1, 4($sp)
add $s1, $t0, 0

# RET w_3  
move $v0, $s1
jr $ra

# FUNC_END aa  
jr $ra

# FUNC_BEGIN main  
main :

# PRINT 19182650 str 
la $a0, str_0
li $v0, 4
syscall

# PRINT \n str 
la $a0, str_1
li $v0, 4
syscall

# ARR_SAVE a_10 0 1
add $s0, $sp, 0
add $a1, $zero, 1
sw $a1, 0($s0)

# ARR_SAVE a_10 1 2
add $a1, $zero, 2
sw $a1, 4($s0)

# ARR_SAVE a_10 2 3
add $a1, $zero, 3
sw $a1, 8($s0)

# ARR_SAVE a_10 3 4
add $a1, $zero, 4
sw $a1, 12($s0)

# ARR_SAVE a_10 4 5
add $a1, $zero, 5
sw $a1, 16($s0)

# ARR_SAVE a_10 5 6
add $a1, $zero, 6
sw $a1, 20($s0)

# ARR_SAVE b_11 0 1
add $s1, $sp, 24
add $a1, $zero, 1
sw $a1, 0($s1)

# ARR_SAVE b_11 1 2
add $a1, $zero, 2
sw $a1, 4($s1)

# ARR_SAVE b_11 2 3
add $a1, $zero, 3
sw $a1, 8($s1)

# ARR_SAVE b_11 3 4
add $a1, $zero, 4
sw $a1, 12($s1)

# ARR_SAVE b_11 4 5
add $a1, $zero, 5
sw $a1, 16($s1)

# ARR_SAVE b_11 5 6
add $a1, $zero, 6
sw $a1, 20($s1)

# PREPARE_CALL aa  
sub $sp, $sp, 176

# ARR_LOAD #Tmp17 b_11 0
lw $t0, 0($s1)

# ARR_SAVE @Arr0 0 #Tmp17
add $s2, $sp, 252
sw $t0, 0($s2)

# ARR_LOAD #Tmp18 b_11 1
lw $t1, 4($s1)

# ARR_SAVE @Arr0 1 #Tmp18
sw $t1, 4($s2)

# ARR_LOAD #Tmp19 b_11 2
lw $t2, 8($s1)

# ARR_SAVE @Arr0 2 #Tmp19
sw $t2, 8($s2)

# ARR_LOAD #Tmp20 b_11 3
lw $t3, 12($s1)

# ARR_SAVE @Arr0 3 #Tmp20
sw $t3, 12($s2)

# ARR_LOAD #Tmp21 b_11 4
lw $t4, 16($s1)

# ARR_SAVE @Arr0 4 #Tmp21
sw $t4, 16($s2)

# ARR_LOAD #Tmp22 b_11 5
lw $t5, 20($s1)

# ARR_SAVE @Arr0 5 #Tmp22
sw $t5, 20($s2)

# PUSH_ARR @Arr0 0 
add $a0, $sp, 252
sw $a0, 0($sp)

# CALL aa  
sw $ra, 76($sp)
sw $s0, 100($sp)
sw $s1, 104($sp)
sw $s2, 108($sp)
sw $t0, 132($sp)
sw $t1, 136($sp)
sw $t2, 140($sp)
sw $t3, 144($sp)
sw $t4, 148($sp)
sw $t5, 152($sp)
jal aa
lw $ra, 76($sp)
lw $s0, 100($sp)
lw $s1, 104($sp)
lw $s2, 108($sp)
lw $t0, 132($sp)
lw $t1, 136($sp)
lw $t2, 140($sp)
lw $t3, 144($sp)
lw $t4, 148($sp)
lw $t5, 152($sp)
add $sp, $sp, 176

# ADD #Tmp23 %RET 0
add $t6, $v0, 0

# ADD d_13 #Tmp23 0
lw $s3, 72($sp)
add $s3, $t6, 0

# MUL #Tmp24 0 3
add $t7, $zero, 0

# ADD #Tmp24 #Tmp24 0
add $t7, $t7, 0

# MUL #Tmp26 0 3
add $t6, $zero, 0

# ADD #Tmp26 #Tmp26 0
add $t6, $t6, 0

# ARR_LOAD #Tmp25 b_11 #Tmp26
sw $t0, 100($sp)
sll $a0, $t6, 2
add $a0, $a0, $s1
lw $t0, 0($a0)

# ARR_SAVE c_12 #Tmp24 #Tmp25
add $s4, $sp, 48
add $a0, $zero, $t7
sll $a0, $a0, 2
add $a0, $a0, $s4
sw $t0, 0($a0)

# MUL #Tmp27 0 3
sw $t1, 104($sp)
add $t1, $zero, 0

# ADD #Tmp27 #Tmp27 1
add $t1, $t1, 1

# MUL #Tmp29 0 3
sw $t2, 108($sp)
add $t2, $zero, 0

# ADD #Tmp29 #Tmp29 1
add $t2, $t2, 1

# ARR_LOAD #Tmp28 b_11 #Tmp29
sw $t3, 112($sp)
sll $a0, $t2, 2
add $a0, $a0, $s1
lw $t3, 0($a0)

# ARR_SAVE c_12 #Tmp27 #Tmp28
add $a0, $zero, $t1
sll $a0, $a0, 2
add $a0, $a0, $s4
sw $t3, 0($a0)

# MUL #Tmp30 0 3
sw $t4, 116($sp)
add $t4, $zero, 0

# ADD #Tmp30 #Tmp30 2
add $t4, $t4, 2

# MUL #Tmp32 0 3
sw $t5, 120($sp)
add $t5, $zero, 0

# ADD #Tmp32 #Tmp32 2
add $t5, $t5, 2

# ARR_LOAD #Tmp31 b_11 #Tmp32
sw $t6, 136($sp)
sll $a0, $t5, 2
add $a0, $a0, $s1
lw $t6, 0($a0)

# ARR_SAVE c_12 #Tmp30 #Tmp31
add $a0, $zero, $t4
sll $a0, $a0, 2
add $a0, $a0, $s4
sw $t6, 0($a0)

# MUL #Tmp33 1 3
sw $t7, 128($sp)
add $t7, $zero, 3

# ADD #Tmp33 #Tmp33 0
add $t7, $t7, 0

# MUL #Tmp35 1 3
sw $t0, 132($sp)
add $t0, $zero, 3

# ADD #Tmp35 #Tmp35 0
add $t0, $t0, 0

# ARR_LOAD #Tmp34 b_11 #Tmp35
sw $t2, 148($sp)
sll $a0, $t0, 2
add $a0, $a0, $s1
lw $t2, 0($a0)

# ARR_SAVE c_12 #Tmp33 #Tmp34
add $a0, $zero, $t7
sll $a0, $a0, 2
add $a0, $a0, $s4
sw $t2, 0($a0)

# MUL #Tmp36 1 3
sw $t1, 140($sp)
add $t1, $zero, 3

# ADD #Tmp36 #Tmp36 1
add $t1, $t1, 1

# MUL #Tmp38 1 3
sw $t3, 144($sp)
add $t3, $zero, 3

# ADD #Tmp38 #Tmp38 1
add $t3, $t3, 1

# ARR_LOAD #Tmp37 b_11 #Tmp38
sw $t5, 160($sp)
sll $a0, $t3, 2
add $a0, $a0, $s1
lw $t5, 0($a0)

# ARR_SAVE c_12 #Tmp36 #Tmp37
add $a0, $zero, $t1
sll $a0, $a0, 2
add $a0, $a0, $s4
sw $t5, 0($a0)

# MUL #Tmp39 1 3
sw $t4, 152($sp)
add $t4, $zero, 3

# ADD #Tmp39 #Tmp39 2
add $t4, $t4, 2

# MUL #Tmp41 1 3
sw $t6, 156($sp)
add $t6, $zero, 3

# ADD #Tmp41 #Tmp41 2
add $t6, $t6, 2

# ARR_LOAD #Tmp40 b_11 #Tmp41
sw $t0, 172($sp)
sll $a0, $t6, 2
add $a0, $a0, $s1
lw $t0, 0($a0)

# ARR_SAVE c_12 #Tmp39 #Tmp40
add $a0, $zero, $t4
sll $a0, $a0, 2
add $a0, $a0, $s4
sw $t0, 0($a0)

# PRINT 1 int 
add $a0, $zero, 1
li $v0, 1
syscall

# PRINT \n str 
la $a0, str_1
li $v0, 4
syscall

# PRINT 2 int 
add $a0, $zero, 2
li $v0, 1
syscall

# PRINT \n str 
la $a0, str_1
li $v0, 4
syscall

# PRINT 3 int 
add $a0, $zero, 3
li $v0, 1
syscall

# PRINT \n str 
la $a0, str_1
li $v0, 4
syscall

# PRINT 4 int 
add $a0, $zero, 4
li $v0, 1
syscall

# PRINT \n str 
la $a0, str_1
li $v0, 4
syscall

# PRINT 5 int 
add $a0, $zero, 5
li $v0, 1
syscall

# PRINT \n str 
la $a0, str_1
li $v0, 4
syscall

# PRINT 6 int 
add $a0, $zero, 6
li $v0, 1
syscall

# PRINT \n str 
la $a0, str_1
li $v0, 4
syscall

# PRINT d_13 int 
move $a0, $s3
li $v0, 1
syscall

# PRINT \n str 
la $a0, str_1
li $v0, 4
syscall

# MUL #Tmp43 0 3
sw $t7, 164($sp)
add $t7, $zero, 0

# ADD #Tmp43 #Tmp43 0
add $t7, $t7, 0

# ARR_LOAD #Tmp42 c_12 #Tmp43
sw $t2, 168($sp)
sll $a0, $t7, 2
add $a0, $a0, $s4
lw $t2, 0($a0)

# PRINT #Tmp42 int 
move $a0, $t2
li $v0, 1
syscall

# PRINT \n str 
la $a0, str_1
li $v0, 4
syscall

# MUL #Tmp45 1 3
sw $t3, 184($sp)
add $t3, $zero, 3

# ADD #Tmp45 #Tmp45 2
add $t3, $t3, 2

# ARR_LOAD #Tmp44 c_12 #Tmp45
sw $t1, 176($sp)
sll $a0, $t3, 2
add $a0, $a0, $s4
lw $t1, 0($a0)

# PRINT #Tmp44 int 
move $a0, $t1
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
