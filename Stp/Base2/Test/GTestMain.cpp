// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Test/TestSuite.h"

APPLICATION_MAIN() {
  using namespace stp;

  TestSuite app(APPLICATION_ARGUMENTS);
  return app.Run();
}
