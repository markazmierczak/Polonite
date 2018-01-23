// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEST_TESTSUITE_H_
#define STP_BASE_TEST_TESTSUITE_H_

#include "Base/App/Application.h"

namespace stp {

class TestSuite : public Application {
 public:
  using Application::Application;

  int Run();

 protected:
  void OnCaptureArguments(CommandLine::Arguments& arguments) override;
  void OnDidInit() override;
};

} // namespace stp

#endif // STP_BASE_TEST_TESTSUITE_H_
