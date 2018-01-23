// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Debug/Debugger.h"

#include <stdlib.h>
#include <windows.h>

namespace stp {

bool Debugger::IsPresent() {
  return ::IsDebuggerPresent() != 0;
}

void Debugger::Break() {
  if (IsDebugUISuppressed())
    _exit(1);

  __debugbreak();
}

} // namespace stp
