	.file	"sum_benchmarks.c"
	.text
	.p2align 4,,15
	.globl	sum_C
	.type	sum_C, @function
sum_C:
.LFB987:
	.cfi_startproc
	testq	%rdi, %rdi
	jle	.L4
	leaq	(%rsi,%rdi,2), %rdx
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
.L3:
	addw	(%rsi), %ax
	addq	$2, %rsi
	cmpq	%rdx, %rsi
	jne	.L3
	rep ret
.L4:
	xorl	%eax, %eax
	ret
	.cfi_endproc
.LFE987:
	.size	sum_C, .-sum_C
	.p2align 4,,15
	.globl	sum_multiple_accum_C
	.type	sum_multiple_accum_C, @function
sum_multiple_accum_C:
.LFB988:
	.cfi_startproc
	testq	%rdi, %rdi
	jle	.L11
	leaq	-2(%rdi,%rdi), %rax
	xorl	%ecx, %ecx
	xorl	%edi, %edi
	xorl	%edx, %edx
	andq	$-8, %rax
	leaq	8(%rsi,%rax), %r8
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
.L10:
	addw	(%rsi), %ax
	addw	2(%rsi), %dx
	addq	$8, %rsi
	addw	-4(%rsi), %di
	addw	-2(%rsi), %cx
	cmpq	%r8, %rsi
	jne	.L10
	addl	%edi, %ecx
	addl	%ecx, %edx
	addl	%edx, %eax
	ret
.L11:
	xorl	%eax, %eax
	ret
	.cfi_endproc
.LFE988:
	.size	sum_multiple_accum_C, .-sum_multiple_accum_C
	.globl	functions
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC0:
	.string	"sum_clang6_O: simple C compiled with clang 6 -O -mavx2"
	.align 8
.LC1:
	.string	"sum_gcc7_O3: simple C compiled with GCC7 -O3 -mavx2"
	.align 8
.LC2:
	.string	"sum_C: simple C compiled on this machine with settings in Makefile"
	.align 8
.LC3:
	.string	"sum_simple: simple ASM implementation"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC4:
	.string	"sum_unrolled2"
.LC5:
	.string	"sum_unrolled4"
.LC6:
	.string	"sum_multiple_accum_C"
.LC7:
	.string	"sum_multiple_accum"
	.data
	.align 32
	.type	functions, @object
	.size	functions, 144
functions:
	.quad	sum_clang6_O
	.quad	.LC0
	.quad	sum_gcc7_O3
	.quad	.LC1
	.quad	sum_C
	.quad	.LC2
	.quad	sum_simple
	.quad	.LC3
	.quad	sum_unrolled2
	.quad	.LC4
	.quad	sum_unrolled4
	.quad	.LC5
	.quad	sum_multiple_accum_C
	.quad	.LC6
	.quad	sum_multiple_accum
	.quad	.LC7
	.quad	0
	.quad	0
	.ident	"GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-44)"
	.section	.note.GNU-stack,"",@progbits
