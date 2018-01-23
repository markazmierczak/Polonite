// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This is a cross platform interface for helper functions related to
// debuggers.  You should use this to test if you're running under a debugger,
// and if you would like to yield (breakpoint) into the debugger.

#ifndef STP_BASE_DEBUG_DEBUGGER_H_
#define STP_BASE_DEBUG_DEBUGGER_H_

#include "Base/Export.h"
#include "Base/Type/Attributes.h"

namespace stp {

class BASE_EXPORT Debugger {
  STATIC_ONLY(Debugger);
 public:
  // Waits wait_seconds seconds for a debugger to attach to the current process.
  // When silent is false, an exception is thrown when a debugger is detected.
  static bool WaitFor(int wait_seconds, bool silent);

  // Returns true if the given process is being run under a debugger.
  static bool IsPresent();

  // Break into the debugger, assumes a debugger is present.
  static void Break();

  // Used in test code, this controls whether showing dialogs and breaking into
  // the debugger is suppressed for debug errors, even in debug mode (normally
  // release mode doesn't do this stuff --  this is controlled separately).
  // Normally UI is not suppressed.  This is normally used when running automated
  // tests where we want a crash rather than a dialog or a debugger.
  static void SetSuppressDebugUI(bool suppress) { g_debug_ui_suppressed_ = suppress; }
  static bool IsDebugUISuppressed() { return g_debug_ui_suppressed_; }

  NEVER_INLINE static const void* GetProgramCounter();

 private:
  static bool g_debug_ui_suppressed_;
};

} // namespace stp

#endif // STP_BASE_DEBUG_DEBUGGER_H_
