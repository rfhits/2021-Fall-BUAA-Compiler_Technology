.data
	var1: .word	3

	array2:	.space	40

.text
	li $t0 2
	sw $t0, array2+0
	sw $t0 var1

ADD:
	lw $s0 -4($sp)
	lw $s1 -8($sp)
	add $t0 $s0 $1
	add $v0 $s0 0
	
MAIN:
	