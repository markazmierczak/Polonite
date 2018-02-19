// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/System/Environment.h"

#include "Base/Test/GTest.h"
#include "Base/Test/PlatformTest.h"

typedef PlatformTest EnvironmentTest;

namespace stp {

TEST_F(EnvironmentTest, GetVar) {
  // Every setup should have non-empty PATH...
  String env_value;
  ASSERT_TRUE(Environment::tryGet("PATH", env_value));
  EXPECT_NE(StringSpan(""), env_value);
}

TEST_F(EnvironmentTest, HasVar) {
  // Every setup should have PATH...
  EXPECT_TRUE(Environment::Has("PATH"));
}

TEST_F(EnvironmentTest, SetVar) {
  StringSpan FooUpper = "FOO";
  StringSpan FooLower = "foo";
  EXPECT_TRUE(Environment::TrySet(FooUpper, FooLower));

  // Now verify that the environment has the new variable.
  EXPECT_TRUE(Environment::Has(FooUpper));

  String var_value;
  ASSERT_TRUE(Environment::tryGet(FooUpper, var_value));
  EXPECT_EQ(FooLower, var_value);
}

TEST_F(EnvironmentTest, UnsetVar) {
  const char FooUpper[] = "FOO";
  const char FooLower[] = "foo";
  // First set some environment variable.
  EXPECT_TRUE(Environment::TrySet(FooUpper, FooLower));

  // Now verify that the environment has the new variable.
  EXPECT_TRUE(Environment::Has(FooUpper));

  // Finally verify that the environment variable was erased.
  EXPECT_TRUE(Environment::TryUnset(FooUpper));

  // And check that the variable has been unset.
  EXPECT_FALSE(Environment::Has(FooUpper));
}

} // namespace stp
