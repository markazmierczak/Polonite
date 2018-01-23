// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_COMPILER_CPU_H_
#define STP_BASE_COMPILER_CPU_H_

#define CPU(x) (((_STP_CPU) & (_STP_CPU_##x)) == (_STP_CPU_##x))

#define _STP_CPU_X86_FAMILY (1 << 0)
#define _STP_CPU_ARM_FAMILY (1 << 1)

#define _STP_CPU_32BIT (1 << 8)
#define _STP_CPU_64BIT (1 << 8)

#define _STP_CPU_X86_32 _STP_CPU_X86_FAMILY | _STP_CPU_32BIT
#define _STP_CPU_X86_64 _STP_CPU_X86_FAMILY | _STP_CPU_64BIT
#define _STP_CPU_ARM32  _STP_CPU_ARM_FAMILY | _STP_CPU_32BIT
#define _STP_CPU_ARM64  _STP_CPU_ARM_FAMILY | _STP_CPU_64BIT

// Processor architecture detection. For more info on what's defined, see:
//   http://msdn.microsoft.com/en-us/library/b0084kay.aspx
//   http://www.agner.org/optimize/calling_conventions.pdf
//   or with gcc, run: "echo | gcc -E -dM -"
#if defined(_M_X64) || defined(__x86_64__)
# define _STP_CPU _STP_CPU_X86_64
#elif defined(_M_IX86) || defined(__i386__)
# define _STP_CPU _STP_CPU_X86_32
#elif defined(_M_ARM) || defined(__ARMEL__)
# define _STP_CPU _STP_CPU_ARM32
#elif defined(_M_ARM64) || defined(__aarch64__)
# define _STP_CPU _STP_CPU_ARM64
#else
# error "please add support for your architecture"
#endif

#endif // STP_BASE_COMPILER_CPU_H_
