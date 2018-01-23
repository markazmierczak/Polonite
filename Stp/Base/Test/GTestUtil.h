// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_TEST_GTESTUTIL_H_
#define STP_BASE_TEST_GTESTUTIL_H_

#include "Base/Compiler/Os.h"
#include "Base/Test/GTest.h"

// EXPECT/ASSERT_ASSERT_DEATH is intended to replace EXPECT/ASSERT_DEBUG_DEATH
// when the death is expected to be caused by a ASSERT. Contrary to
// EXPECT/ASSERT_DEBUG_DEATH however, it doesn't execute the statement in release
// builds as ASSERTs are intended to catch things that should never
// happen and as such executing the statement results in undefined behavior
// (|statement| is compiled in unsupported configurations nonetheless).
// Death tests misbehave on Android.
#if !defined(NDEBUG) && defined(GTEST_HAS_DEATH_TEST) && !OS(ANDROID)

#define EXPECT_ASSERT_DEATH(statement) EXPECT_DEATH(statement, "Assertion")
#define ASSERT_ASSERT_DEATH(statement) ASSERT_DEATH(statement, "Assertion")

#else

// Macro copied from GTestDeathTestInternal.h as it's (1) internal for now
// and (2) only defined if !GTEST_HAS_DEATH_TEST which is only a subset of the
// conditions in which it's needed here.
# define GTEST_UNSUPPORTED_DEATH_TEST(statement, regex, terminator) \
    GTEST_AMBIGUOUS_ELSE_BLOCKER_ \
    if (::testing::internal::AlwaysTrue()) { \
      GTEST_LOG_(WARNING) \
          << "Death tests are not supported on this platform.\n" \
          << "Statement '" #statement "' cannot be verified."; \
    } else if (::testing::internal::AlwaysFalse()) { \
      ::testing::internal::RE::PartialMatch(".*", (regex)); \
      GTEST_SUPPRESS_UNREACHABLE_CODE_WARNING_BELOW_(statement); \
      terminator; \
    } else \
      ::testing::Message()

#define EXPECT_ASSERT_DEATH(statement) \
    GTEST_UNSUPPORTED_DEATH_TEST(statement, "assertion", )
#define ASSERT_ASSERT_DEATH(statement) \
    GTEST_UNSUPPORTED_DEATH_TEST(statement, "assertion", return)

#endif // !defined(NDEBUG) && defined(GTEST_HAS_DEATH_TEST) && !OS(ANDROID)

#endif // STP_BASE_TEST_GTESTUTIL_H_
