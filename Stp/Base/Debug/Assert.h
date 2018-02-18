// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_DEBUG_ASSERT_H_
#define STP_BASE_DEBUG_ASSERT_H_

#include "Base/Export.h"
#include "Base/Type/Attributes.h"

namespace stp {

class TextWriter;

#if defined(NDEBUG) && !defined(HAVE_ASSERT_ALWAYS_ON)
# define ASSERT_IS_ON() 0
#else
# define ASSERT_IS_ON() 1
#endif

BASE_EXPORT void AssertFail(const char* file, int line, const char* expr);
BASE_EXPORT void AssertFail(const char* file, int line, const char* expr, const char* msg);
BASE_EXPORT void AssertCrash();

BASE_EXPORT TextWriter& AssertPrint(const char* file, int line, const char* expr);
BASE_EXPORT void AssertWrapUp(TextWriter& out);

} // namespace stp

#if ASSERT_IS_ON()

# define ASSERT(expr, ...) \
  ((expr) \
   ? static_cast<void>(0) \
   : stp::AssertFail(__FILE__, __LINE__, #expr, ##__VA_ARGS__))

#define ASSERT_WITH_MESSAGE ASSERT

# define ASSERT_UNUSED(expr, var) ASSERT(expr)
# define ASSUME(cond, ...) ASSERT(cond, ##__VA_ARGS__)
# define UNREACHABLE(...) ASSERT(false); __VA_ARGS__
#else
# define ASSERT(expr, ...) static_cast<void>(0)
# define ASSERT_WITH_MESSAGE ASSERT
# define ASSERT_UNUSED(expr, var) ALLOW_UNUSED_LOCAL(var)
# define ASSUME(cond, ...) BUILTIN_ASSUME(cond)

# if COMPILER(MSVC) && !COMPILER(CLANG)
# define UNREACHABLE(...) BUILTIN_UNREACHABLE()
# else
# define UNREACHABLE(...) BUILTIN_UNREACHABLE(); __VA_ARGS__
#endif

#endif // !ASSERT_IS_ON

#endif // STP_BASE_DEBUG_ASSERT_H_
