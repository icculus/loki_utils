
/* Code to detect certain processor features */

#ifndef _loki_cpuinfo_h
#define _loki_cpuinfo_h

/* There is some interesting code in MGL, but I'm not sure of the license
   issues.
 */

#define CPU_HAS_MMX	0x0001		/* Pentium II MMX */
#define CPU_HAS_EMMX	0x0002		/* Cyrix extended MMX */
#define CPU_HAS_3DNOW	0x0004		/* AMD 3DNow! */
#define CPU_HAS_SSE	0x0008		/* Pentium III SSE */

extern int loki_getcpuflags(void);

#endif /* _loki_cpuinfo_h */
