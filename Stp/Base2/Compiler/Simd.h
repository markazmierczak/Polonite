// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_COMPILER_SIMD_H_
#define STP_BASE_COMPILER_SIMD_H_

#include "Base/Compiler/Cpu.h"

#define _STP_CPU_SIMD_LEVEL_SSE1     10
#define _STP_CPU_SIMD_LEVEL_SSE2     20
#define _STP_CPU_SIMD_LEVEL_SSE3     30
#define _STP_CPU_SIMD_LEVEL_SSSE3    31
#define _STP_CPU_SIMD_LEVEL_SSE41    41
#define _STP_CPU_SIMD_LEVEL_SSE42    42
#define _STP_CPU_SIMD_LEVEL_AVX      51
#define _STP_CPU_SIMD_LEVEL_AVX2     52

#define _STP_CPU_SIMD_LEVEL_NEON     (1 << 8)

#if CPU(X86_FAMILY)

#define _STP_CPU_SIMD_LEVEL_MASK (0xFF << 0)

// Are we in GCC?
// These checks must be done in descending order to ensure we set the highest
// available SSE level.
#if defined(__AVX2__)
# define _STP_CPU_SIMD_LEVEL _STP_CPU_SIMD_LEVEL_AVX2
#elif defined(__AVX__)
# define _STP_CPU_SIMD_LEVEL _STP_CPU_SIMD_LEVEL_AVX
#elif defined(__SSE4_2__)
# define _STP_CPU_SIMD_LEVEL _STP_CPU_SIMD_LEVEL_SSE42
#elif defined(__SSE4_1__)
# define _STP_CPU_SIMD_LEVEL _STP_CPU_SIMD_LEVEL_SSE41
#elif defined(__SSSE3__)
# define _STP_CPU_SIMD_LEVEL _STP_CPU_SIMD_LEVEL_SSSE3
#elif defined(__SSE3__)
# define _STP_CPU_SIMD_LEVEL _STP_CPU_SIMD_LEVEL_SSE3
#elif defined(__SSE2__)
# define _STP_CPU_SIMD_LEVEL _STP_CPU_SIMD_LEVEL_SSE2
#endif

#ifndef _STP_CPU_SIMD_LEVEL
// Are we in VisualStudio?
// These checks must be done in descending order to ensure we set the highest
// available SSE level. 64-bit intel guarantees at least SSE2 support.
#if defined(_M_X64) || defined(_M_AMD64)
# define _STP_CPU_SIMD_LEVEL _STP_CPU_SIMD_LEVEL_SSE2
#elif defined (_M_IX86_FP)
# if _M_IX86_FP >= 2
#  define _STP_CPU_SIMD_LEVEL _STP_CPU_SIMD_LEVEL_SSE2
# elif _M_IX86_FP == 1
#  define _STP_CPU_SIMD_LEVEL _STP_CPU_SIMD_LEVEL_SSE1
# endif
#endif

#endif

#endif // CPU(X86_FAMILY)

// All 64-bit ARM chips have NEON. Many 32-bit ARM chips do too.
#if CPU(ARM_FAMILY)
# define _STP_CPU_SIMD_LEVEL_MASK (0xFF << 8)

# if defined(__ARM_NEON)
#  define _STP_CPU_SIMD_LEVEL _STP_CPU_SIMD_LEVEL_NEON
# endif
#endif

#define CPU_SIMD(version) \
	((_STP_CPU_SIMD_LEVEL_##version & _STP_CPU_SIMD_LEVEL_MASK) && _STP_CPU_SIMD_LEVEL >= _STP_CPU_SIMD_LEVEL_##version)

#if CPU(X86_FAMILY)
#if CPU_SIMD(SSE2)
# include <emmintrin.h>
#endif
#if CPU_SIMD(SSE3)
# include <pmmintrin.h>
#endif
#if CPU_SIMD(SSSE3)
# include <tmmintrin.h>
#endif
#if CPU_SIMD(SSE41)
# include <smmintrin.h>
#endif
#if CPU_SIMD(SSE42)
# include <nmmintrin.h>
#endif
#if CPU_SIMD(AVX)
# include <immintrin.h>
#endif
#elif CPU(ARM_FAMILY)
#if CPU_SIMD(NEON)
# include <arm_neon.h>
#endif
#endif

#endif // STP_BASE_COMPILER_SIMD_H_
