// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/Debugger.h"

#include "Base/Thread/Thread.h"

#if COMPILER(MSVC)
#include <intrin.h>
#endif

namespace stp {

bool Debugger::g_debug_ui_suppressed_ = false;

/**
 * @class Debugger
 * This is a cross platform interface for helper functions related to debuggers.
 * You should use this to test if you're running under a debugger,
 * and if you would like to yield (breakpoint) into the debugger.
 */

/**
 * @fn Debugger::isPresent
 * Returns true if the given process is being run under a debugger.
 */

//
/**
 * Waits @a wait_seconds seconds for a debugger to attach to the current process.
 * @param silent When false, an exception is thrown when a debugger is detected.
 */
bool Debugger::waitFor(int wait_seconds, bool silent) {
  #if OS(ANDROID)
  // The pid from which we know which process to attach to are not output by
  // android ddms, so we have to print it out explicitly.
  LOG(INFO, "Debugger::WaitFor(pid={})", getpid());
  #endif
  for (int i = 0; i < wait_seconds * 10; ++i) {
    if (Debugger::isPresent()) {
      if (!silent)
        Debugger::breakpoint();
      return true;
    }
    ThisThread::SleepFor(TimeDelta::FromMilliseconds(100));
  }
  return false;
}

const void* Debugger::getProgramCounter() {
  #if COMPILER(MSVC)
  return _ReturnAddress();
  #elif COMPILER(GCC)
  return __builtin_extract_return_addr(__builtin_return_address(0));
  #else
  return nullptr;
  #endif
}

/**
 * @fn Debugger::setSuppressDebugUi
 * Used in test code, this controls whether showing dialogs and breaking into
 * the debugger is suppressed for debug errors, even in debug mode (normally
 * release mode doesn't do this stuff -- this is controlled separately).
 * Normally UI is not suppressed. This is normally used when running automated
 * tests where we want a crash rather than a dialog or a debugger.
 */

} // namespace stp
