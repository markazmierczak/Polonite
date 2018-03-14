// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_ATTRIBUTES_H_
#define STP_BASE_TYPE_ATTRIBUTES_H_

#include "Base/Compiler/Config.h"

#define ALLOW_UNUSED_LOCAL(x) (void)x

#if COMPILER(GCC) || COMPILER(CLANG)
#define ALLOW_UNUSED_TYPE __attribute__((unused))
#else
#define ALLOW_UNUSED_TYPE
#endif

#if COMPILER(GCC) || COMPILER(CLANG)
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define WARN_UNUSED_RESULT
#endif

template<typename T>
inline void ignoreResult(const T&) noexcept {}

#if COMPILER(GCC)
#define NEVER_INLINE __attribute__((noinline))
#elif COMPILER(MSVC)
#define NEVER_INLINE __declspec(noinline)
#endif

#if COMPILER(GCC)
#define ALWAYS_INLINE inline __attribute__((always_inline))
#elif COMPILER(MSVC)
#define ALWAYS_INLINE __forceinline
#endif

#if COMPILER(MSVC)
#define RESTRICT __restrict
#define RETURNS_NOALIAS __attribute__((__malloc__))
#elif COMPILER(GCC)
#define RESTRICT __restrict__
#define RETURNS_NOALIAS __declspec(restrict)
#endif

#if COMPILER(GCC)
#define BUILTIN_UNREACHABLE() __builtin_unreachable()
#elif COMPILER(MSVC)
#define BUILTIN_UNREACHABLE() __assume(false)
#endif

#if COMPILER(CLANG)
#define BUILTIN_ASSUME(cond) __builtin_assume(cond)
#elif COMPILER(GCC)
#define BUILTIN_ASSUME(cond) do { if (!(cond)) __builtin_unreachable(); } while (0)
#elif COMPILER(MSVC)
#define BUILTIN_ASSUME(cond) __assume(cond)
#endif

#if COMPILER(GCC)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif

#if COMPILER(MSVC)
#define ALIGNAS(byte_alignment) __declspec(align(byte_alignment))
#elif COMPILER(GCC)
#define ALIGNAS(byte_alignment) __attribute__((aligned(byte_alignment)))
#endif

#if COMPILER(MSVC)
# define DEPRECATED(decl, message) __declspec(deprecated(message)) decl
#elif COMPILER(GCC)
# if COMPILER(CLANG)
#  define DEPRECATED(decl, message) decl __attribute__((deprecated(message)))
# else
#  define DEPRECATED(decl, message) decl __attribute__((deprecated))
# endif
#endif

#if COMPILER(MSVC)
#define EMPTY_BASES_LAYOUT __declspec(empty_bases)
#else
#define EMPTY_BASES_LAYOUT
#endif

// A macro to disallow the copy constructor and operator= functions.
// This should be used in the private: declarations for a class.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete; \
  void operator=(const TypeName&) = delete

#define STATIC_ONLY(TypeName) \
  TypeName() = delete; \
  DISALLOW_COPY_AND_ASSIGN(TypeName)

#endif // STP_BASE_TYPE_ATTRIBUTES_H_
