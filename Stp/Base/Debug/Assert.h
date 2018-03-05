// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_DEBUG_ASSERT_H_
#define STP_BASE_DEBUG_ASSERT_H_

#include "Base/Type/Attributes.h"

namespace stp {

class StringSpan;

#if !defined(ASSERT_IS_ON)
#define ASSERT_IS_ON !defined(NDEBUG)
#endif
#if !defined(STP_ENABLE_DEBUG_MESSAGES)
#define STP_ENABLE_DEBUG_MESSAGES !defined(NDEBUG)
#endif

#if ASSERT_IS_ON
BASE_EXPORT [[noreturn]] void panic(const char* file, int line, const char* expr, const char* msg);
#define PANIC(...) stp::panic(__FILE__, __LINE__, nullptr, ##__VA_ARGS__)
#define PANIC_IF(expr, ...) if (UNLIKELY(expr)) { stp::panic(__FILE__, __LINE__, #expr, ##__VA_ARGS__); }
#else
BASE_EXPORT [[noreturn]] void panic();
BASE_EXPORT [[noreturn]] void panic(const char* msg);
#if STP_ENABLE_DEBUG_MESSAGES
#define PANIC stp::panic
#define PANIC_IF(expr, ...) if (UNLIKELY(expr)) { PANIC(##__VA_ARGS__); }
#else
#define PANIC(...) panic()
#define PANIC_IF(expr, ...) if (UNLIKELY(expr)) { stp::panic(); }
#endif
#endif

} // namespace stp

#if ASSERT_IS_ON
# define ASSERT(expr, ...) PANIC_IF(!(expr), "assertion failed: " __VA_ARGS__)
# define ASSERT_UNUSED(expr, var) ASSERT(expr)
# define ASSUME(cond, ...) ASSERT(cond, ##__VA_ARGS__)
# define UNREACHABLE(...) ASSERT(false); __VA_ARGS__
#else
# define ASSERT(expr, ...) static_cast<void>(0)
# define ASSERT_UNUSED(expr, var) ALLOW_UNUSED_LOCAL(var)
# define ASSUME(cond, ...) BUILTIN_ASSUME(cond)

# if COMPILER(MSVC) && !COMPILER(CLANG)
# define UNREACHABLE(...) BUILTIN_UNREACHABLE()
# else
# define UNREACHABLE(...) BUILTIN_UNREACHABLE(); __VA_ARGS__
#endif

#endif // !ASSERT_IS_ON

#endif // STP_BASE_DEBUG_ASSERT_H_
