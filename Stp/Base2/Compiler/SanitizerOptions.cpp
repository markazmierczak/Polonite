// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file contains the default options for various compiler-based dynamic
// tools.

#include "Base/Compiler/Os.h"
#include "Base/Compiler/Sanitizer.h"

#if SANITIZER(ADDRESS) && OS(DARWIN)
#include <crt_externs.h>  // for _NSGetArgc, _NSGetArgv
#include <string.h>
#endif // SANITIZER(ADDRESS) && OS(MAC)

#if SANITIZER(ADDRESS) || SANITIZER(LEAK) || SANITIZER(MEMORY) || SANITIZER(THREAD) || SANITIZER(UNDEFINED)
// Functions returning default options are declared weak in the tools' runtime
// libraries. To make the linker pick the strong replacements for those
// functions from this module, we explicitly force its inclusion by passing
// -Wl,-u_sanitizer_options_link_helper
extern "C"
void _sanitizer_options_link_helper() { }

// The callbacks we define here will be called from the sanitizer runtime, but
// aren't referenced from the executable. We must ensure that those
// callbacks are not sanitizer-instrumented, and that they aren't stripped by
// the linker.
#define SANITIZER_HOOK_ATTRIBUTE                                           \
  extern "C"                                                               \
  __attribute__((no_sanitize("address", "memory", "thread", "undefined"))) \
  __attribute__((visibility("default")))                                   \
  __attribute__((used))
#endif

#if SANITIZER(ADDRESS)
// Default options for AddressSanitizer in various configurations:
//   malloc_context_size=5 - limit the size of stack traces collected by ASan
//     for each malloc/free by 5 frames. These stack traces tend to accumulate
//     very fast in applications using JIT (v8 in Chrome's case), see
//     https://code.google.com/p/address-sanitizer/issues/detail?id=177
//   symbolize=1 - enable in-process symbolization.
//   legacy_pthread_cond=1 - run in the libpthread 2.2.5 compatibility mode to
//     work around libGL.so using the obsolete API, see
//     http://crbug.com/341805. This may break if pthread_cond_t objects are
//     accessed by both instrumented and non-instrumented binaries (e.g. if
//     they reside in shared memory). This option is going to be deprecated in
//     upstream AddressSanitizer and must not be used anywhere except the
//     official builds.
//   check_printf=1 - check the memory accesses to printf (and other formatted
//     output routines) arguments.
//   use_sigaltstack=1 - handle signals on an alternate signal stack. Useful
//     for stack overflow detection.
//   strip_path_prefix=/../../ - prefixes up to and including this
//     substring will be stripped from source file paths in symbolized reports
//   fast_unwind_on_fatal=1 - use the fast (frame-pointer-based) stack unwinder
//     to print error reports. V8 doesn't generate debug info for the JIT code,
//     so the slow unwinder may not work properly.
//   detect_stack_use_after_return=1 - use fake stack to delay the reuse of
//     stack allocations and detect stack-use-after-return errors.
#if OS(LINUX)
// Default AddressSanitizer options for buildbots and non-official builds.
const char* AsanDefaultOptions =
    "symbolize=1 check_printf=1 use_sigaltstack=1 "
    "detect_leaks=0 strip_path_prefix=/../../ fast_unwind_on_fatal=1 "
    "detect_stack_use_after_return=1 ";
#elif OS(DARWIN)
const char* AsanDefaultOptions =
    "check_printf=1 use_sigaltstack=1 "
    "strip_path_prefix=/../../ fast_unwind_on_fatal=1 "
    "detect_stack_use_after_return=1 detect_odr_violation=0 ";
#endif // OS(LINUX)

#if OS(LINUX) || OS(DARWIN)
SANITIZER_HOOK_ATTRIBUTE const char* __asan_default_options() {
  return AsanDefaultOptions;
}

extern "C" char ASanDefaultSuppressions[];

SANITIZER_HOOK_ATTRIBUTE const char* __asan_default_suppressions() {
  return ASanDefaultSuppressions;
}
#endif // OS(LINUX) || OS(DARWIN)
#endif // SANITIZER(ADDRESS)

#if SANITIZER(THREAD) && OS(LINUX)
// Default options for ThreadSanitizer in various configurations:
//   detect_deadlocks=1 - enable deadlock (lock inversion) detection.
//   second_deadlock_stack=1 - more verbose deadlock reports.
//   report_signal_unsafe=0 - do not report async-signal-unsafe functions
//     called from signal handlers.
//   report_thread_leaks=0 - do not report unjoined threads at the end of
//     the program execution.
//   print_suppressions=1 - print the list of matched suppressions.
//   history_size=7 - make the history buffer proportional to 2^7 (the maximum
//     value) to keep more stack traces.
//   strip_path_prefix=/../../ - prefixes up to and including this
//     substring will be stripped from source file paths in symbolized reports.
const char TsanDefaultOptions[] =
    "detect_deadlocks=1 second_deadlock_stack=1 report_signal_unsafe=0 "
    "report_thread_leaks=0 print_suppressions=1 history_size=7 "
    "strict_memcmp=0 strip_path_prefix=/../../ ";

SANITIZER_HOOK_ATTRIBUTE const char* __tsan_default_options() {
  return TsanDefaultOptions;
}

extern "C" char TsanDefaultSuppressions[];

SANITIZER_HOOK_ATTRIBUTE const char* __tsan_default_suppressions() {
  return TsanDefaultSuppressions;
}

#endif // SANITIZER(THREAD) && OS(LINUX)

#if SANITIZER(MEMORY)
// Default options for MemorySanitizer:
//   intercept_memcmp=0 - do not detect uninitialized memory in memcmp() calls.
//     Pending cleanup, see http://crbug.com/523428
//   strip_path_prefix=/../../ - prefixes up to and including this
//     substring will be stripped from source file paths in symbolized reports.
const char MsanDefaultOptions[] =
    "intercept_memcmp=0 strip_path_prefix=/../../ ";

SANITIZER_HOOK_ATTRIBUTE const char* __msan_default_options() {
  return MsanDefaultOptions;
}

#endif // SANITIZER(MEMORY)

#if SANITIZER(LEAK)
// Default options for LeakSanitizer:
//   print_suppressions=1 - print the list of matched suppressions.
//   strip_path_prefix=/../../ - prefixes up to and including this
//     substring will be stripped from source file paths in symbolized reports.
const char LsanDefaultOptions[] =
    "print_suppressions=1 strip_path_prefix=/../../ ";

SANITIZER_HOOK_ATTRIBUTE const char* __lsan_default_options() {
  return LsanDefaultOptions;
}

extern "C" char LSanDefaultSuppressions[];

SANITIZER_HOOK_ATTRIBUTE const char* __lsan_default_suppressions() {
  return LSanDefaultSuppressions;
}

#endif // SANITIZER(LEAK)

#if SANITIZER(UNDEFINED)
// Default options for UndefinedBehaviorSanitizer:
//   print_stacktrace=1 - print the stacktrace when UBSan reports an error.
const char UbsanDefaultOptions[] =
    "print_stacktrace=1 strip_path_prefix=/../../ ";

SANITIZER_HOOK_ATTRIBUTE const char* __ubsan_default_options() {
  return UbsanDefaultOptions;
}

#endif // SANITIZER(UNDEFINED)
