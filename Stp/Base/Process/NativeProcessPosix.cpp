// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Process/NativeProcess.h"

#include <unistd.h>

namespace stp {

NativeProcessId NativeProcess::GetCurrentId() {
  return getpid();
}

NativeProcessHandle NativeProcess::GetCurrentHandle() {
  return NativeProcess::GetCurrentId();
}

} // namespace stp
