// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Debug/Debugger.h"

#include "Base/Thread/Thread.h"

#if COMPILER(MSVC)
#include <intrin.h>
#endif

namespace stp {

bool Debugger::g_debug_ui_suppressed_ = false;

bool Debugger::WaitFor(int wait_seconds, bool silent) {
  #if OS(ANDROID)
  // The pid from which we know which process to attach to are not output by
  // android ddms, so we have to print it out explicitly.
  LOG(INFO, "Debugger::WaitFor(pid={})", getpid());
  #endif
  for (int i = 0; i < wait_seconds * 10; ++i) {
    if (Debugger::IsPresent()) {
      if (!silent)
        Debugger::Break();
      return true;
    }
    ThisThread::SleepFor(TimeDelta::FromMilliseconds(100));
  }
  return false;
}

const void* Debugger::GetProgramCounter() {
  #if COMPILER(MSVC)
  return _ReturnAddress();
  #elif COMPILER(GCC)
  return __builtin_extract_return_addr(__builtin_return_address(0));
  #else
  return nullptr;
  #endif
}

} // namespace stp
