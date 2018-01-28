// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_PREFETCH_H_
#define STP_BASE_MEMORY_PREFETCH_H_

#include "Base/Compiler/Simd.h"

#if CPU_SIMD(SSE1)
# include <xmmintrin.h>
# define PREFETCH(ptr) _mm_prefetch(reinterpret_cast<const char*>(ptr), _MM_HINT_T0)
# define WRITE_PREFETCH(ptr) _mm_prefetch(reinterpret_cast<const char*>(ptr), _MM_HINT_T0)
#elif COMPILER(GCC)
# define PREFETCH(ptr)       __builtin_prefetch(ptr)
# define WRITE_PREFETCH(ptr) __builtin_prefetch(ptr, 1)
#else
# define PREFETCH(ptr)
# define WRITE_PREFETCH(ptr)
#endif

#endif // STP_BASE_MEMORY_PREFETCH_H_
