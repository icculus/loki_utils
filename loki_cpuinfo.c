
/* Code to detect certain processor features */

#include "loki_cpuinfo.h"


/* Heheh, inline ASM - Sam */
int loki_getcpuflags(void)
{
	int flags;

	flags = 0;

#if defined(__GNUC__) && defined(i386)
/* Taken with thanks from mmx.h:

        MultiMedia eXtensions GCC interface library for IA32.

        THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
        EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
        LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
        AND FITNESS FOR ANY PARTICULAR PURPOSE.

        1997-98 by H. Dietz and R. Fisher

	Bug fixes and SSE detection by Sam Lantinga
*/
    {
	/* Returns 1 if MMX instructions are supported,
	   3 if Cyrix MMX and Extended MMX instructions are supported
	   5 if AMD MMX and 3DNow! instructions are supported
	   0 if hardware does not support any of these
	*/
	__asm__ __volatile__ (
		"pushl %%ebx\n\t"
		"pushl %%ecx\n\t"
		"pushl %%edx\n\t"

		/* See if CPUID instruction is supported ... */
		/* ... Get copies of EFLAGS into eax and ecx */
		"pushf\n\t"
		"popl %%eax\n\t"
		"movl %%eax, %%ecx\n\t"

		/* ... Toggle the ID bit in one copy and store */
		/*     to the EFLAGS reg */
		"xorl $0x200000, %%eax\n\t"
		"push %%eax\n\t"
		"popf\n\t"

		/* ... Get the (hopefully modified) EFLAGS */
		"pushf\n\t"
		"popl %%eax\n\t"

		/* ... Compare and test result */
		"xorl %%eax, %%ecx\n\t"
		"testl $0x200000, %%ecx\n\t"
		"jz NotSupported\n\t"		/* Nothing supported */

		/* Get standard CPUID information, and
		       go to a specific vendor section */
		"movl $0, %%eax\n\t"
		"cpuid\n\t"

		/* Check for Intel */
		"cmpl $0x756e6547, %%ebx\n\t"
		"jne TryAMD\n\t"
		"cmpl $0x49656e69, %%edx\n\t"
		"jne TryAMD\n\t"
		"cmpl $0x6c65746e, %%ecx\n"
		"jne TryAMD\n\t"
		"jmp Intel\n\t"

		/* Check for AMD */
		"\nTryAMD:\n\t"
		"cmpl $0x68747541, %%ebx\n\t"
		"jne TryCyrix\n\t"
		"cmpl $0x69746e65, %%edx\n\t"
		"jne TryCyrix\n\t"
		"cmpl $0x444d4163, %%ecx\n"
		"jne TryCyrix\n\t"
		"jmp AMD\n\t"

		/* Check for Cyrix */
		"\nTryCyrix:\n\t"
		"cmpl $0x69727943, %%ebx\n\t"
		"jne NotSupported\n\t"
		"cmpl $0x736e4978, %%edx\n\t"
		"jne NotSupported\n\t"
		"cmpl $0x64616574, %%ecx\n\t"
		"jne NotSupported\n\t"
		/* Drop through to Cyrix... */


		/* Cyrix Section */
		/* See if extended CPUID is supported */
		"movl $0x80000000, %%eax\n\t"
		"cpuid\n\t"
		"cmpl $0x80000000, %%eax\n\t"
		"jl MMXtest\n\t"	/* Try standard CPUID instead */

		/* Extended CPUID supported, so get extended features */
		"movl $0x80000001, %%eax\n\t"
		"cpuid\n\t"
		"testl $0x00800000, %%eax\n\t"	/* Test for MMX */
		"jz NotSupported\n\t"		/* MMX not supported */
		"movl $1, %0\n\n\t"		/* MMX Supported */
		"testl $0x01000000, %%eax\n\t"	/* Test for Ext'd MMX */
		"jz Return\n\t"
		"addl $2, %0\n\n\t"		/* EMMX also supported */
		"jmp Return\n\t"


		/* AMD Section */
		"AMD:\n\t"

		/* See if extended CPUID is supported */
		"movl $0x80000000, %%eax\n\t"
		"cpuid\n\t"
		"cmpl $0x80000000, %%eax\n\t"
		"jl MMXtest\n\t"	/* Try standard CPUID instead */

		/* Extended CPUID supported, so get extended features */
		"movl $0x80000001, %%eax\n\t"
		"cpuid\n\t"
		"testl $0x00800000, %%edx\n\t"	/* Test for MMX */
		"jz NotSupported\n\t"		/* MMX not supported */
		"movl $1, %0\n\n\t"		/* MMX Supported */
		"testl $0x80000000, %%edx\n\t"	/* Test for 3DNow! */
		"jz Return\n\t"
		"addl $4, %0\n\n\t"		/* 3DNow! also supported */
		"jmp Return\n\t"


		/* Intel Section */
		"Intel:\n\t"

		/* Check for MMX */
		"MMXtest:\n\t"
		"movl $1, %%eax\n\t"
		"cpuid\n\t"
		"testl $0x00800000, %%edx\n\t"	/* Test for MMX */
		"jz NotSupported\n\t"		/* MMX Not supported */
		"movl $1, %0\n\n\t"		/* MMX Supported */
		"testl $0x02000000, %%edx\n\t"	/* Test for SSE */
		"jz Return\n\t"			/* SSE Not supported */
		"addl $8, %0\n\n\t"		/* SSE Supported */
		"jmp Return\n\t"

		/* Nothing supported */
		"NotSupported:\n\t"
		"movl $0, %0\n\n\t"

		"Return:\n\t"
		"popl %%edx\n\t"
		"popl %%ecx\n\t"
		"popl %%ebx\n\t"
		: "=a" (flags)
		: /* no input */
/*
		: "ebx", "ecx", "edx"
*/
		: "memory"
	);
    }
#endif /* GCC and x86 */

    return flags;
}


#ifdef TEST_MAIN

#include <stdio.h>

int main(int argc, char *argv[])
{
	int cpu_flags;

	cpu_flags = loki_getcpuflags();

	printf("CPU flags: 0x%4.4x", cpu_flags);
	if ( cpu_flags & CPU_HAS_MMX ) {
		printf(" MMX");
	}
	if ( cpu_flags & CPU_HAS_EMMX ) {
		printf(" (exended)");
	}
	if ( cpu_flags & CPU_HAS_3DNOW ) {
		printf(" 3DNow!");
	}
	if ( cpu_flags & CPU_HAS_SSE ) {
		printf(" SSE");
	}
	printf("\n");
	exit(0);
}

#endif /* TEST_MAIN */
