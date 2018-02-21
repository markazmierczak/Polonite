// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_DEBUG_STACKTRACE_H_
#define STP_BASE_DEBUG_STACKTRACE_H_

#include "Base/Compiler/Os.h"
#include "Base/Type/Formattable.h"

#if OS(POSIX)
#include <unistd.h>
#endif

#if OS(WIN)
struct _EXCEPTION_POINTERS;
struct _CONTEXT;
#endif

namespace stp {

BASE_EXPORT void FormatSymbol(TextWriter& out, void* pc);

class BASE_EXPORT StackTrace {
 public:
  // Enables stack dump to console output on exception and signals.
  // When enabled, the process will quit immediately. This is meant to be used in
  // unit tests only! This is not thread-safe: only call from main thread.
  static bool EnableInProcessDump();

  // Creates a stacktrace from the current location.
  StackTrace();

  // Creates a stacktrace from an existing array of instruction
  // pointers (such as returned by GetAddresses()).  |count| will be
  // trimmed to |MaxTraces|.
  StackTrace(void* const* trace, int count);

  #if OS(WIN)
  // Creates a stacktrace for an exception.
  // Note: this function will throw an import not found (StackWalk64) exception
  // on system without dbghelp 5.1.
  StackTrace(_EXCEPTION_POINTERS* exception_pointers);
  StackTrace(const _CONTEXT* context);
  #endif

  // Copying and assignment are allowed.

  // Gets an array of instruction pointer values. |*count| will be set to the
  // number of elements in the returned array.
  void* const* GetAddresses(int* count) const;

  // Prints the stack trace to stderr.
  void PrintToConsole() const;

  friend void format(TextWriter& out, const StackTrace& x, const StringSpan& opts) {
    x.formatImpl(out, opts);
  }
  friend TextWriter& operator<<(TextWriter& out, const StackTrace& x) {
    x.FormatSymbols(out); return out;
  }

 private:
  #if OS(WIN)
  void InitTrace(const _CONTEXT* context_record);
  #endif

  void formatImpl(TextWriter& out, const StringSpan& opts) const;
  void FormatSymbols(TextWriter& out) const;
  void FormatAddresses(TextWriter& out) const;

  // From http://msdn.microsoft.com/en-us/library/bb204633.aspx,
  // the sum of FramesToSkip and FramesToCapture must be less than 63,
  // so set it to 62. Even if on POSIX it could be a larger value, it usually
  // doesn't give much more information.
  static constexpr int MaxTraces = 62;

  void* trace_[MaxTraces];

  // The number of valid frames in |trace_|.
  int count_;
};

#if OS(POSIX)
void DemangleSymbols(TextWriter& out, StringSpan mangled);
#endif

} // namespace stp

#endif // STP_BASE_DEBUG_STACKTRACE_H_
