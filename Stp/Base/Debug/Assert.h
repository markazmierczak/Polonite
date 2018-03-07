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

#define _STP_PANIC_GET_3TH_ARG(a, b, c, ...) c

#if STP_ENABLE_DEBUG_MESSAGES
BASE_EXPORT [[noreturn]] void panic(const char* file, int line, const char* msg = "", const char* msg2 = "");
#define PANIC(...) stp::panic(__FILE__, __LINE__, ##__VA_ARGS__)
#else
BASE_EXPORT [[noreturn]] void panic();
#define PANIC(...) stp::panic()
#endif

#define _STP_PANIC_IF2(expr, msg) if (UNLIKELY(expr)) { PANIC(#expr, msg); }
#define _STP_PANIC_IF1(expr) _STP_PANIC_IF2(expr, "explicit")
#define _STP_PANIC_IF_SELECT(...) _STP_PANIC_GET_3TH_ARG(__VA_ARGS__, _STP_PANIC_IF2, _STP_PANIC_IF1)
#define PANIC_IF(...) _STP_PANIC_IF_SELECT(__VA_ARGS__)(__VA_ARGS__)

} // namespace stp

#if ASSERT_IS_ON
# define _STP_ASSERT2(expr, msg) if (UNLIKELY(!(expr))) { PANIC("assertion failed: " #expr, msg); }
# define UNREACHABLE(...) ASSERT(false); __VA_ARGS__
#else
# define _STP_ASSERT2(expr, msg) do { (void)sizeof(expr); } while(0)
# if COMPILER(MSVC) && !COMPILER(CLANG)
# define UNREACHABLE(...) BUILTIN_UNREACHABLE()
# else
# define UNREACHABLE(...) BUILTIN_UNREACHABLE(); __VA_ARGS__
#endif

#endif // !ASSERT_IS_ON

#define _STP_ASSERT1(expr) _STP_ASSERT2(expr, "explicit")
#define _STP_ASSERT_SELECT(...) _STP_PANIC_GET_3TH_ARG(__VA_ARGS__, _STP_ASSERT2, _STP_ASSERT1)
#define ASSERT(...) _STP_ASSERT_SELECT(__VA_ARGS__)(__VA_ARGS__)

#endif // STP_BASE_DEBUG_ASSERT_H_
