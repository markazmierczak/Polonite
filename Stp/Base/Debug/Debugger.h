// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_DEBUG_DEBUGGER_H_
#define STP_BASE_DEBUG_DEBUGGER_H_

#include "Base/Export.h"
#include "Base/Type/Attributes.h"

namespace stp {

class BASE_EXPORT Debugger {
  STATIC_ONLY(Debugger);
 public:
  static bool waitFor(int wait_seconds, bool silent);
  static bool isPresent();
  static void breakpoint();

  static void setSuppressDebugUi(bool suppress) { g_debug_ui_suppressed_ = suppress; }
  static bool isDebugUiSuppressed() { return g_debug_ui_suppressed_; }

  NEVER_INLINE static const void* getProgramCounter();

 private:
  static bool g_debug_ui_suppressed_;
};

} // namespace stp

#endif // STP_BASE_DEBUG_DEBUGGER_H_
