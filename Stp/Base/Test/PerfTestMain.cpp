// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Test/PerfTestSuite.h"

APPLICATION_MAIN() {
  using namespace stp;

  PerfTestSuite app(APPLICATION_ARGUMENTS);
  return app.Run();
}
