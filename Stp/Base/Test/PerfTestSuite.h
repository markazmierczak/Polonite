// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEST_PERFTESTSUITE_H_
#define STP_BASE_TEST_PERFTESTSUITE_H_

#include "Base/Test/TestSuite.h"

namespace stp {

class PerfTestSuite : public TestSuite {
 public:
  using TestSuite::TestSuite;

 protected:
  void OnDidInit() override;
  void OnWillFini() override;
};

} // namespace stp

#endif // STP_BASE_TEST_PERFTESTSUITE_H_
