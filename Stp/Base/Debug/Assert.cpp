// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/Assert.h"

#include "Base/Debug/Alias.h"
// FIXME #include "Base/Debug/Console.h"
#include "Base/Debug/Debugger.h"
#include "Base/Debug/StackTrace.h"

// FIXME use console for write
#include <stdio.h>
#include <stdlib.h>

namespace stp {

[[noreturn]] static void crash() {
  static int Dummy;
  debugAlias(&Dummy);

  // Crash the process to generate a dump.
  Debugger::breakpoint();
  abort();
}

#if STP_ENABLE_DEBUG_MESSAGES
void panic(const char* file, int line, const char* msg, const char* msg2) {
  fprintf(stderr, "panic! %s:%d: %s, %s\n", file, line, msg, msg2);
  crash();
}
#else
void panic() {
  fprintf(stderr, "panic!\n");
  crash();
}
#endif // STP_ENABLE_DEBUG_MESSAGES

void assertPrint(const char* file, int line, const char* msg) {
  fprintf(stderr, "%s:%d: %s\n", file, line, msg);
  /* FIXME ConsoleWriter& out = Console::err();

  out.setLogLevel(LogLevelFATAL);

  #if !defined(NDEBUG)
  // Include a stack trace on a fatal, unless a debugger is attached.
  bool being_debugged = Debugger::isPresent();
  if (!being_debugged) {
    out << "Stack Trace: \n";
    // FIXME format(out, StackTrace());
  }
  #endif

  out.setForegroundColor(ConsoleColor::Red);
  out << "Assertion failed: ";
  out.resetColors();

  out << '"';
  out << StringSpan::fromCString(expr);
  out << "\" at ";
  out << StringSpan::fromCString(file) << ':'  << line << '\n';

  out.setForegroundColor(ConsoleColor::Red);
  out << '\n';
  out.resetColors();*/
}

} // namespace stp
