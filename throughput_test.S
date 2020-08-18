/* memchr/wmemchr optimized with AVX2.
   Copyright (C) 2017-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#define L(a) _ ## a
#define ENTRY(a) a:
#define RDX_LP rdx
#define END(z)

# ifndef MEMCHR
#  define MEMCHR	MEMCHR_IMPL
# endif

# ifdef USE_AS_WMEMCHR
#  define VPCMPEQ	vpcmpeqd
# else
#  define VPCMPEQ	vpcmpeqb
# endif

# ifndef VZEROUPPER
#  define VZEROUPPER	vzeroupper
# endif

# define VEC_SIZE 64 

	.text
	.globl  MEMCHR_IMPL
ENTRY (MEMCHR)
# ifndef USE_AS_RAWMEMCHR
	/* Check for zero length.  */
	test	%RDX_LP, %RDX_LP
	jz	L(null)
# endif
	movl	%edi, %ecx
	/* Broadcast CHAR to YMM0.  */
	vmovd	%esi, %xmm0
# ifdef USE_AS_WMEMCHR
	shl	$2, %RDX_LP
	vpbroadcastd %xmm0, %ymm0
# else
#  ifdef __ILP32__
	/* Clear the upper 32 bits.  */
	movl	%edx, %edx
#  endif
	vpbroadcastb %xmm0, %zmm0
# endif
	/* Check if we may cross page boundary with one vector load.  */
	andl	$(2 * VEC_SIZE - 1), %ecx
	cmpl	$VEC_SIZE, %ecx
	ja	L(cros_page_boundary)

	/* Check the first VEC_SIZE bytes.  */
	VPCMPEQ (%rdi), %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax

# ifndef USE_AS_RAWMEMCHR
	jnz	L(first_vec_x0_check)
	/* Adjust length and check the end of data.  */
	subq	$VEC_SIZE, %rdx
	jbe	L(zero)
# else
	jnz	L(first_vec_x0)
# endif

	/* Align data for aligned loads in the loop.  */
	addq	$VEC_SIZE, %rdi
	andl	$(VEC_SIZE - 1), %ecx
	andq	$-VEC_SIZE, %rdi

# ifndef USE_AS_RAWMEMCHR
	/* Adjust length.  */
	addq	%rcx, %rdx

	subq	$(VEC_SIZE * 4), %rdx
	jbe	L(last_4x_vec_or_less)
# endif
	jmp	L(more_4x_vec)

	.p2align 4
L(cros_page_boundary):
	andl	$(VEC_SIZE - 1), %ecx
	andq	$-VEC_SIZE, %rdi
	VPCMPEQ (%rdi), %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	/* Remove the leading bytes.  */
	sarl	%cl, %eax
	testl	%eax, %eax
	jz	L(aligned_more)
	tzcntl	%eax, %eax
# ifndef USE_AS_RAWMEMCHR
	/* Check the end of data.  */
	cmpq	%rax, %rdx
	jbe	L(zero)
# endif
	addq	%rdi, %rax
	addq	%rcx, %rax
	VZEROUPPER
	ret

	.p2align 4
L(aligned_more):
# ifndef USE_AS_RAWMEMCHR
        /* Calculate "rdx + rcx - VEC_SIZE" with "rdx - (VEC_SIZE - rcx)"
	   instead of "(rdx + rcx) - VEC_SIZE" to void possible addition
	   overflow.  */
	negq	%rcx
	addq	$VEC_SIZE, %rcx

	/* Check the end of data.  */
	subq	%rcx, %rdx
	jbe	L(zero)
# endif

	addq	$VEC_SIZE, %rdi

# ifndef USE_AS_RAWMEMCHR
	subq	$(VEC_SIZE * 4), %rdx
	jbe	L(last_4x_vec_or_less)
# endif

L(more_4x_vec):
	/* Check the first 4 * VEC_SIZE.  Only one VEC_SIZE at a time
	   since data is only aligned to VEC_SIZE.  */
	VMOVDQA64 (%rdi), %zmm1
	VMOVDQA64 VEC_SIZE(%rdi), %zmm2
	VMOVDQA64 (VEC_SIZE * 2)(%rdi), %zmm3
	VMOVDQA64 (VEC_SIZE * 3)(%rdi), %zmm4

	addq	$(VEC_SIZE * 4), %rdi

# ifndef USE_AS_RAWMEMCHR
	subq	$(VEC_SIZE * 4), %rdx
	jbe	L(last_4x_vec_or_less)
# endif

	/* Align data to 4 * VEC_SIZE.  */
	movq	%rdi, %rcx
	andl	$(4 * VEC_SIZE - 1), %ecx
	andq	$-(4 * VEC_SIZE), %rdi

# ifndef USE_AS_RAWMEMCHR
	/* Adjust length.  */
	addq	%rcx, %rdx
# endif

	.p2align 4
L(loop_4x_vec):
	/* Compare 4 * VEC at a time forward.  */
	VMOVDQA64 (%rdi), %zmm1

	addq	$(VEC_SIZE), %rdi

	subq	$(VEC_SIZE), %rdx
	ja	L(loop_4x_vec)

L(last_4x_vec_or_less):
	/* Less than 4 * VEC and aligned to VEC_SIZE.  */
	addl	$(VEC_SIZE * 2), %edx
	VMOVDQA64 (%rdi), %zmm1
	VMOVDQA64 VEC_SIZE(%rdi), %zmm2
	VMOVDQA64 (VEC_SIZE * 2)(%rdi), %zmm3
	subl	$VEC_SIZE, %edx
	VMOVDQA64 (VEC_SIZE * 3)(%rdi), %zmm4

	ret

	.p2align 4
; L(last_2x_vec):
; 	addl	$(VEC_SIZE * 2), %edx
; 	VPCMPEQ (%rdi), %ymm0, %ymm1
; 	vpmovmskb %ymm1, %eax
; 	testl	%eax, %eax

; 	jnz	L(first_vec_x0_check)
; 	subl	$VEC_SIZE, %edx
; 	jle	L(zero)

; 	VPCMPEQ VEC_SIZE(%rdi), %ymm0, %ymm1
; 	vpmovmskb %ymm1, %eax
; 	testl	%eax, %eax
; 	jnz	L(first_vec_x1_check)
; 	xorl	%eax, %eax
; 	VZEROUPPER
; 	ret

; 	.p2align 4
; L(first_vec_x0_check):
; 	tzcntl	%eax, %eax
; 	/* Check the end of data.  */
; 	cmpq	%rax, %rdx
; 	jbe	L(zero)
; 	addq	%rdi, %rax
; 	VZEROUPPER
; 	ret

; 	.p2align 4
; L(first_vec_x1_check):
; 	tzcntl	%eax, %eax
; 	/* Check the end of data.  */
; 	cmpq	%rax, %rdx
; 	jbe	L(zero)
; 	addq	$VEC_SIZE, %rax
; 	addq	%rdi, %rax
; 	VZEROUPPER
; 	ret

; 	.p2align 4
; L(first_vec_x2_check):
; 	tzcntl	%eax, %eax
; 	/* Check the end of data.  */
; 	cmpq	%rax, %rdx
; 	jbe	L(zero)
; 	addq	$(VEC_SIZE * 2), %rax
; 	addq	%rdi, %rax
; 	VZEROUPPER
; 	ret

; 	.p2align 4
; L(first_vec_x3_check):
; 	tzcntl	%eax, %eax
; 	/* Check the end of data.  */
; 	cmpq	%rax, %rdx
; 	jbe	L(zero)
; 	addq	$(VEC_SIZE * 3), %rax
; 	addq	%rdi, %rax
; 	VZEROUPPER
; 	ret

; 	.p2align 4
; L(zero):
; 	VZEROUPPER
; L(null):
; 	xorl	%eax, %eax
; 	ret

; 	.p2align 4
; L(first_vec_x0):
; 	tzcntl	%eax, %eax
; 	addq	%rdi, %rax
; 	VZEROUPPER
; 	ret

; 	.p2align 4
; L(first_vec_x1):
; 	tzcntl	%eax, %eax
; 	addq	$VEC_SIZE, %rax
; 	addq	%rdi, %rax
; 	VZEROUPPER
; 	ret

; 	.p2align 4
; L(first_vec_x2):
; 	tzcntl	%eax, %eax
; 	addq	$(VEC_SIZE * 2), %rax
; 	addq	%rdi, %rax
; 	VZEROUPPER
; 	ret

; 	.p2align 4
; L(4x_vec_end):
; 	vpmovmskb %ymm1, %eax
; 	testl	%eax, %eax
; 	jnz	L(first_vec_x0)
; 	vpmovmskb %ymm2, %eax
; 	testl	%eax, %eax
; 	jnz	L(first_vec_x1)
; 	vpmovmskb %ymm3, %eax
; 	testl	%eax, %eax
; 	jnz	L(first_vec_x2)
; 	vpmovmskb %ymm4, %eax
; 	testl	%eax, %eax
; L(first_vec_x3):
; 	tzcntl	%eax, %eax
; 	addq	$(VEC_SIZE * 3), %rax
; 	addq	%rdi, %rax
; 	VZEROUPPER
; 	ret

END (MEMCHR)
