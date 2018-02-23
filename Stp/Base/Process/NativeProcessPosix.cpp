// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Process/NativeProcess.h"

#include <unistd.h>

namespace stp {

NativeProcessId NativeProcess::getCurrentId() {
  return getpid();
}

NativeProcessHandle NativeProcess::getCurrentHandle() {
  return NativeProcess::getCurrentId();
}

} // namespace stp
