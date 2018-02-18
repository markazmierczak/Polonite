// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/Debugger.h"

#include <stdlib.h>
#include <windows.h>

namespace stp {

bool Debugger::isPresent() {
  return ::IsDebuggerPresent() != 0;
}

void Debugger::breakpoint() {
  if (isDebugUiSuppressed()) {
    _exit(1);
  }
  __debugbreak();
}

} // namespace stp
