// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/Assert.h"

#include "Base/Debug/Alias.h"
#include "Base/Debug/Console.h"
#include "Base/Debug/Debugger.h"
#include "Base/Debug/StackTrace.h"
#include "Base/Text/FormatMany.h"

#include <stdlib.h>

namespace stp {

void assertWrapUp(TextWriter& out_base) {
  ConsoleWriter& out = static_cast<ConsoleWriter&>(out_base);
  out << '\n';
  out.resetColors();
  assertCrash();
}

void assertFail(const char* file, int line, const char* expr) {
  TextWriter& out = assertPrint(file, line, expr);
  assertWrapUp(out);
}

void assertFail(const char* file, int line, const char* expr, const char* msg) {
  TextWriter& out = assertPrint(file, line, expr);
  out << MakeSpanFromNullTerminated(msg);
  assertWrapUp(out);
}

TextWriter& assertPrint(const char* file, int line, const char* expr) {
  ConsoleWriter& out = Console::err();

  out.setLogLevel(LogLevelFATAL);

  #if !defined(NDEBUG)
  // Include a stack trace on a fatal, unless a debugger is attached.
  bool being_debugged = Debugger::isPresent();
  if (!being_debugged) {
    out << "Stack Trace: \n";
    // FIXME Format(out, StackTrace());
  }
  #endif

  out.setForegroundColor(ConsoleColor::Red);
  out << "Assertion failed: ";
  out.resetColors();

  out << '"';
  out << MakeSpanFromNullTerminated(expr);
  out << "\" at ";
  out << MakeSpanFromNullTerminated(file) << ':'  << line << '\n';

  out.setForegroundColor(ConsoleColor::Red);
  return out;
}

void assertCrash() {
  static int Dummy;
  debugAlias(&Dummy);

  // Crash the process to generate a dump.
  Debugger::breakpoint();
}

} // namespace stp
