// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/Assert.h"

#include "Base/Debug/Alias.h"
#include "Base/Debug/Console.h"
#include "Base/Debug/Debugger.h"
#include "Base/Debug/StackTrace.h"
#include "Base/Text/Format.h"

#include <stdlib.h>

namespace stp {

void AssertWrapUp(TextWriter& out_base) {
  ConsoleWriter& out = static_cast<ConsoleWriter&>(out_base);
  out << EndOfLine;
  out.ResetColors();
  AssertCrash();
}

void AssertFail(const char* file, int line, const char* expr) {
  TextWriter& out = AssertPrint(file, line, expr);
  AssertWrapUp(out);
}

void AssertFail(const char* file, int line, const char* expr, const char* msg) {
  TextWriter& out = AssertPrint(file, line, expr);
  out << MakeSpanFromNullTerminated(msg);
  AssertWrapUp(out);
}

TextWriter& AssertPrint(const char* file, int line, const char* expr) {
  ConsoleWriter& out = Console::Err();

  out.SetLogLevel(LogLevelFATAL);

  #if !defined(NDEBUG)
  // Include a stack trace on a fatal, unless a debugger is attached.
  bool being_debugged = Debugger::IsPresent();
  if (!being_debugged) {
    out << "Stack Trace: " << EndOfLine;
    // FIXME Format(out, StackTrace());
  }
  #endif

  out.SetForegroundColor(ConsoleColor::Red);
  out.Write("Assertion failed: ");
  out.ResetColors();

  out << '"';
  out << MakeSpanFromNullTerminated(expr);
  out << "\" at ";
  out << MakeSpanFromNullTerminated(file) << ':'  << line << EndOfLine;

  out.SetForegroundColor(ConsoleColor::Red);
  return out;
}

void AssertCrash() {
  static int Dummy;
  DebugAlias(&Dummy);

  // Crash the process to generate a dump.
  Debugger::Break();
}

} // namespace stp
