	.file	"min_benchmarks.c"
	.text
	.globl	min_C
	.type	min_C, @function
min_C:
.LFB4741:
	.cfi_startproc
	testq	%rdi, %rdi
	jle	.L4
	movl	$0, %edx
	movl	$32767, %eax
.L3:
	movslq	%edx, %rcx
	movzwl	(%rsi,%rcx,2), %ecx
	cmpw	%cx, %ax
	cmovg	%ecx, %eax
	addl	$1, %edx
	movslq	%edx, %rcx
	cmpq	%rdi, %rcx
	jl	.L3
	ret
.L4:
	movl	$32767, %eax
	ret
	.cfi_endproc
.LFE4741:
	.size	min_C, .-min_C
	.globl	min_AVX
	.type	min_AVX, @function
min_AVX:
.LFB4742:
	.cfi_startproc
	leaq	8(%rsp), %r10
	.cfi_def_cfa 10, 0
	andq	$-32, %rsp
	pushq	-8(%r10)
	pushq	%rbp
	.cfi_escape 0x10,0x6,0x2,0x76,0
	movq	%rsp, %rbp
	pushq	%r10
	.cfi_escape 0xf,0x3,0x76,0x78,0x6
	vmovdqu	(%rsi), %ymm0
	cmpq	$16, %rdi
	jle	.L7
	movl	$16, %eax
.L8:
	movslq	%eax, %rdx
	vpminsw	(%rsi,%rdx,2), %ymm0, %ymm0
	addl	$16, %eax
	movslq	%eax, %rdx
	cmpq	%rdi, %rdx
	jl	.L8
.L7:
	vmovdqa	%ymm0, -48(%rbp)
	leaq	-48(%rbp), %rdx
	leaq	-16(%rbp), %rsi
	movl	$32767, %eax
.L9:
	movzwl	(%rdx), %ecx
	cmpl	%ecx, %eax
	cmovg	%ecx, %eax
	addq	$2, %rdx
	cmpq	%rdx, %rsi
	jne	.L9
	popq	%r10
	.cfi_def_cfa 10, 0
	popq	%rbp
	leaq	-8(%r10), %rsp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4742:
	.size	min_AVX, .-min_AVX
	.globl	functions
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"C (local)"
.LC1:
	.string	"min with AVX"
	.data
	.align 32
	.type	functions, @object
	.size	functions, 48
functions:
	.quad	min_C
	.quad	.LC0
	.quad	min_AVX
	.quad	.LC1
	.quad	0
	.quad	0
	.ident	"GCC: (GNU) 7.1.0"
	.section	.note.GNU-stack,"",@progbits
