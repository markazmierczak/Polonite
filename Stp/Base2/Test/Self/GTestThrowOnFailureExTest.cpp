// Copyright 2009, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Tests Google Test's throw-on-failure mode with exceptions enabled.

#include "Base/Test/GTest.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdexcept>

// Prints the given failure message and exits the program with
// non-zero.  We use this instead of a Google Test assertion to
// indicate a failure, as the latter is been tested and cannot be
// relied on.
void Fail(const char* msg) {
  printf("FAILURE: %s\n", msg);
  fflush(stdout);
  exit(1);
}

// Tests that an assertion failure throws a subclass of
// std::runtime_error.
void TestFailureThrowsRuntimeError() {
  testing::GTEST_FLAG(throw_on_failure) = true;

  // A successful assertion shouldn't throw.
  try {
    EXPECT_EQ(3, 3);
  } catch(...) {
    Fail("A successful assertion wrongfully threw.");
  }

  // A failed assertion should throw a subclass of std::runtime_error.
  try {
    EXPECT_EQ(2, 3) << "Expected failure";
  } catch(const std::runtime_error& e) {
    if (strstr(e.what(), "Expected failure") != NULL)
      return;

    printf("%s",
           "A failed assertion did throw an exception of the right type, "
           "but the message is incorrect.  Instead of containing \"Expected "
           "failure\", it is:\n");
    Fail(e.what());
  } catch(...) {
    Fail("A failed assertion threw the wrong type of exception.");
  }
  Fail("A failed assertion should've thrown but didn't.");
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);

  // We want to ensure that people can use Google Test assertions in
  // other testing frameworks, as long as they initialize Google Test
  // properly and set the thrown-on-failure mode.  Therefore, we don't
  // use Google Test's constructs for defining and running tests
  // (e.g. TEST and RUN_ALL_TESTS) here.

  TestFailureThrowsRuntimeError();
  return 0;
}
