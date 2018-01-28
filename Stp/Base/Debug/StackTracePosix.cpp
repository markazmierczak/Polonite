// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Debug/StackTrace.h"

#include "Base/Debug/Console.h"
#include "Base/Debug/Debugger.h"
#include "Base/Debug/Log.h"
#include "Base/Memory/Allocate.h"
#include "Base/Memory/OwnPtr.h"
#include "Base/Posix/EintrWrapper.h"
#include "Base/Text/Format.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#if OS(DARWIN)
#include <AvailabilityMacros.h>
#endif

// Include cstddef to pull in c++config.
#include <cstddef>

#if defined(__GLIBCXX__)
#include <cxxabi.h>
#endif
#if !defined(__UCLIBC__)
#include <execinfo.h>
#endif

namespace stp {

#if defined(__GLIBCXX__)
// The prefix used for mangled symbols, per the Itanium C++ ABI:
// http://www.codesourcery.com/cxx-abi/abi.html#mangling
constexpr StringSpan MangledSymbolPrefix = "_Z";

constexpr StringSpan SymbolCharacters =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
#endif // defined(__GLIBCXX__)

// Demangles C++ symbols in the given text. Example:
//
// "out/Debug/base_unittests(_ZN10StackTraceC1Ev+0x20) [0x817778c]"
// =>
// "out/Debug/base_unittests(StackTrace::StackTrace()+0x20) [0x817778c]"
void DemangleSymbols(TextWriter& out, StringSpan mangled) {
  // Note: code in this function is NOT async-signal safe (String uses
  // malloc internally).
  #if defined(__GLIBCXX__) && !defined(__UCLIBC__)

  while (!mangled.IsEmpty()) {
    // Look for the start of a mangled symbol, from search_from.
    int mangled_start = mangled.IndexOf(MangledSymbolPrefix);
    if (mangled_start < 0) {
      out.Write(mangled);
      break; // Mangled symbol not found.
    }
    if (mangled_start > 0) {
      out.Write(mangled.GetSlice(0, mangled_start));
      mangled.RemovePrefix(mangled_start);
    }

    // Look for the end of the mangled symbol.
    int mangled_end = mangled.IndexOfAnyBut(SymbolCharacters);
    if (mangled_end < 0)
      mangled_end = mangled.size();

    String mangled_symbol(mangled.GetSlice(0, mangled_end));

    // Try to demangle the mangled symbol candidate.
    int status = 0;
    OwnPtr<char> demangled_symbol_cstr(
        abi::__cxa_demangle(ToNullTerminated(mangled_symbol), nullptr, 0, &status));
    if (status == 0) {  // Demangling is successful.
      auto demangled_symbol = MakeSpanFromNullTerminated(demangled_symbol_cstr.get());
      out.Write(demangled_symbol);

      mangled.RemovePrefix(mangled_end);
    } else {
      // Failed to demangle.  Retry after the "_Z" we just found.
      mangled.RemovePrefix(2);
    }
  }
  #endif // defined(__GLIBCXX__) && !defined(__UCLIBC__)
}

static StringSpan GetSignalName(int signal, int code) {
  if (signal == SIGBUS) {
    if (code == BUS_ADRALN)
      return "BUS_ADRALN";
    if (code == BUS_ADRERR)
      return "BUS_ADRERR";
    if (code == BUS_OBJERR)
      return "BUS_OBJERR";
    return "<unknown>";
  }
  if (signal == SIGFPE) {
    if (code == FPE_FLTDIV)
      return "FPE_FLTDIV";
    if (code == FPE_FLTINV)
      return "FPE_FLTINV";
    if (code == FPE_FLTOVF)
      return "FPE_FLTOVF";
    if (code == FPE_FLTRES)
      return "FPE_FLTRES";
    if (code == FPE_FLTSUB)
      return "FPE_FLTSUB";
    if (code == FPE_FLTUND)
      return "FPE_FLTUND";
    if (code == FPE_INTDIV)
      return "FPE_INTDIV";
    if (code == FPE_INTOVF)
      return "FPE_INTOVF";
    return "<unknown>";
  }
  if (signal == SIGILL) {
    if (code == ILL_BADSTK)
      return "ILL_BADSTK";
    if (code == ILL_COPROC)
      return "ILL_COPROC";
    if (code == ILL_ILLOPN)
      return "ILL_ILLOPN";
    if (code == ILL_ILLADR)
      return "ILL_ILLADR";
    if (code == ILL_ILLTRP)
      return "ILL_ILLTRP";
    if (code == ILL_PRVOPC)
      return "ILL_PRVOPC";
    if (code == ILL_PRVREG)
      return "ILL_PRVREG";
    return "<unknown>";
  }
  if (signal == SIGSEGV) {
    if (code == SEGV_MAPERR)
      return "SEGV_MAPERR";
    if (code == SEGV_ACCERR)
      return "SEGV_ACCERR";
  }
  return "<unknown>";
}

static void StackDumpSignalHandler(int signal, siginfo_t* info, void* void_context) {
  if (Debugger::IsPresent())
    Debugger::Break();

  ConsoleWriter& out = Console::Err();
  out.WriteAscii("Received signal ");
  out.Write(signal);

  out.Write(GetSignalName(signal, info->si_code));

  if (signal == SIGBUS || signal == SIGFPE ||
      signal == SIGILL || signal == SIGSEGV) {
    Format(out, info->si_addr);
  }
  out.WriteLine();

  #if defined(CFI_ENFORCEMENT)
  if (signal == SIGILL && info->si_code == ILL_ILLOPN) {
    out.WriteAscii(
        "CFI: Most likely a control flow integrity violation; for more "
        "information see:\n");
    out.WriteAscii("https://www.chromium.org/developers/testing/control-flow-integrity\n");
  }
  #endif

  out.Flush();

  StackTrace stack_trace;
  #if 1
  stack_trace.PrintToConsole();
  #else
  int count;
  auto addresses = stack_trace.GetAddresses(&count);
  int stderr_fd = 2;
  backtrace_symbols_fd(addresses, count, stderr_fd);
  #endif

  #if OS(MAC)
  if (::signal(signal, SIG_DFL) == SIG_ERR)
    _exit(1);
  #else
  _exit(1);
  #endif // OS(MAC)
}

static void WarmUpBacktrace() {
  // Warm up stack trace infrastructure. It turns out that on the first
  // call glibc initializes some internal data structures using pthread_once,
  // and even backtrace() can call malloc(), leading to hangs.
  StackTrace stack_trace;
}

#if !OS(LINUX) && !OS(DARWIN)
void FormatSymbol(TextWriter& out, void* pc) {
  OwnPtr<char*[], FreeDeleter> trace_symbols(backtrace_symbols(&pc, 1));
  if (trace_symbols) {
    auto trace_symbol = MakeSpanFromNullTerminated(trace_symbols[0]);
    DemangleSymbols(out, trace_symbol);
  }
}

void StackTrace::FormatSymbols(TextWriter& out) const {
  // Below part is async-signal unsafe (uses malloc), so execute it only
  // when we are not executing the signal handler.
  OwnPtr<char*[], FreeDeleter> trace_symbols(backtrace_symbols(trace_, count_));
  if (trace_symbols) {
    for (int i = 0; i < count_; ++i) {
      auto trace_symbol = MakeSpanFromNullTerminated(trace_symbols.get()[i]);
      DemangleSymbols(out, trace_symbol);
      out.WriteLine();
    }
  }
}
#endif // OS(*)

bool StackTrace::EnableInProcessDump() {
  // When running in an application, our code typically expects SIGPIPE
  // to be ignored.  Therefore, when testing that same code, it should run
  // with SIGPIPE ignored as well.
  struct sigaction sigpipe_action;
  memset(&sigpipe_action, 0, sizeof(sigpipe_action));
  sigpipe_action.sa_handler = SIG_IGN;
  sigemptyset(&sigpipe_action.sa_mask);
  bool success = (sigaction(SIGPIPE, &sigpipe_action, NULL) == 0);

  // Avoid hangs during backtrace initialization, see above.
  WarmUpBacktrace();

  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_flags = SA_RESETHAND | SA_SIGINFO;
  action.sa_sigaction = &StackDumpSignalHandler;
  sigemptyset(&action.sa_mask);

  success &= (sigaction(SIGILL, &action, NULL) == 0);
  success &= (sigaction(SIGABRT, &action, NULL) == 0);
  success &= (sigaction(SIGFPE, &action, NULL) == 0);
  success &= (sigaction(SIGBUS, &action, NULL) == 0);
  success &= (sigaction(SIGSEGV, &action, NULL) == 0);
  // On Linux, SIGSYS is reserved by the kernel for seccomp-bpf sandboxing.
  #if !OS(LINUX)
  success &= (sigaction(SIGSYS, &action, NULL) == 0);
  #endif

  return success;
}

StackTrace::StackTrace() {
  // NOTE: This code MUST be async-signal safe (it's used by in-process
  // stack dumping signal handler). NO malloc or stdio is allowed here.

  #if !defined(__UCLIBC__)
  // Though the backtrace API man page does not list any possible negative
  // return values, we take no chance.
  count_ = backtrace(trace_, MaxTraces);
  #else
  count_ = 0;
  #endif
}

} // namespace stp
