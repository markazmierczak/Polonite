// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Debug/Debugger.h"

#include "Base/Compiler/Cpu.h"
#include "Base/Compiler/Os.h"
#include "Base/Debug/Alias.h"
#include "Base/Posix/EintrWrapper.h"
#include "Base/Text/StringSpan.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(__GLIBCXX__)
#include <cxxabi.h>
#endif

#if OS(DARWIN)
#include <AvailabilityMacros.h>
#endif

#if OS(DARWIN) || OS(BSD)
#include <sys/sysctl.h>
#endif

#if OS(FREEBSD)
#include <sys/user.h>
#endif

namespace stp {

#if OS(DARWIN) || OS(BSD)

// Based on Apple's recommended method as described in
// http://developer.apple.com/qa/qa2004/qa1361.html
bool Debugger::IsPresent() {
  // NOTE: This code MUST be async-signal safe (it's used by in-process
  // stack dumping signal handler). NO malloc or stdio is allowed here.
  //
  // While some code used below may be async-signal unsafe, note how
  // the result is cached (see |is_set| and |being_debugged| static variables
  // right below). If this code is properly warmed-up early
  // in the start-up process, it should be safe to use later.

  // If the process is sandboxed then we can't use the sysctl, so cache the
  // value.
  static bool is_set = false;
  static bool being_debugged = false;

  if (is_set)
    return being_debugged;

  // Initialize mib, which tells sysctl what info we want.  In this case,
  // we're looking for information about a specific process ID.
  int mib[] = {
    CTL_KERN,
    KERN_PROC,
    KERN_PROC_PID,
    getpid()
  };

  // Caution: struct kinfo_proc is marked __APPLE_API_UNSTABLE.  The source and
  // binary interfaces may change.
  struct kinfo_proc info;
  size_t info_size = sizeof(info);

  int sysctl_result = sysctl(mib, ArraySizeOf(mib), &info, &info_size, NULL, 0);
  ASSERT(sysctl_result == 0);
  if (sysctl_result != 0) {
    is_set = true;
    being_debugged = false;
    return being_debugged;
  }

  // This process is being debugged if the P_TRACED flag is set.
  is_set = true;
  #if OS(FREEBSD)
  being_debugged = (info.ki_flag & P_TRACED) != 0;
  #elif OS(BSD)
  being_debugged = (info.p_flag & P_TRACED) != 0;
  #else
  being_debugged = (info.kp_proc.p_flag & P_TRACED) != 0;
  #endif
  return being_debugged;
}

#elif OS(LINUX) || OS(ANDROID)

// We can look in /proc/self/status for TracerPid.  We are likely used in crash
// handling, so we are careful not to use the heap or have side effects.
// Another option that is common is to try to ptrace yourself, but then we
// can't detach without forking(), and that's not so great.
bool Debugger::IsPresent() {
  // NOTE: This code MUST be async-signal safe (it's used by in-process
  // stack dumping signal handler). NO malloc or stdio is allowed here.

  int status_fd = open("/proc/self/status", O_RDONLY);
  if (status_fd == -1)
    return false;

  // We assume our line will be in the first 1024 characters and that we can
  // read this much all at once.  In practice this will generally be true.
  // This simplifies and speeds up things considerably.
  char buf[1024];

  int num_read = HANDLE_EINTR(read(status_fd, buf, sizeof(buf)));
  if (IGNORE_EINTR(close(status_fd)) < 0)
    return false;

  if (num_read <= 0)
    return false;

  StringSpan status(buf, num_read);
  StringSpan tracer("TracerPid:\t");

  // FIXME support for IndexOfRange
  int pid_index = -1; // IndexOfRange(status, tracer);
  if (pid_index < 0)
    return false;

  // Our pid is 0 without a debugger, assume this for any pid starting with 0.
  pid_index += tracer.size();
  return pid_index < status.size() && status[pid_index] != '0';
}

#else
# error "not implemented"
#endif

// We want to break into the debugger in Debug mode, and cause a crash dump in
// Release mode. Breakpad behaves as follows:
//
// +-------+-----------------+-----------------+
// | OS    | Dump on SIGTRAP | Dump on SIGABRT |
// +-------+-----------------+-----------------+
// | Linux |       N         |        Y        |
// | Mac   |       Y         |        N        |
// +-------+-----------------+-----------------+
//
// Thus we do the following:
// Linux: Debug mode if a debugger is attached, send SIGTRAP; otherwise send
//        SIGABRT
// Mac: Always send SIGTRAP.

#if CPU(ARM32)
# define DEBUG_BREAK_ASM() asm("bkpt 0")
#elif CPU(ARM64)
# define DEBUG_BREAK_ASM() asm("brk 0")
#elif CPU(X86_FAMILY)
# define DEBUG_BREAK_ASM() asm("int3")
#endif

#if defined(NDEBUG) && !OS(DARWIN) && !OS(ANDROID)
# define DEBUG_BREAK() abort()
#elif !OS(DARWIN)
// Though Android has a "helpful" process called debuggerd to catch native
// signals on the general assumption that they are fatal errors. If no debugger
// is attached, we call abort since Breakpad needs SIGABRT to create a dump.
// When debugger is attached, for ARM platform the bkpt instruction appears
// to cause SIGBUS which is trapped by debuggerd, and we've had great
// difficulty continuing in a debugger once we stop from SIG triggered by native
// code, use GDB to set |go| to 1 to resume execution; for X86 platform, use
// "int3" to setup breakpiont and raise SIGTRAP.
//
// On other POSIX architectures, except Mac OS X, we use the same logic to
// ensure that breakpad creates a dump on crashes while it is still possible to
// use a debugger.
static void DebugBreak() {
  if (!Debugger::IsPresent()) {
    abort();
  } else {
    #if defined(DEBUG_BREAK_ASM)
    DEBUG_BREAK_ASM();
    #else
    volatile int go = 0;
    while (!go) {
      ThisThread::Sleep(TimeDelta::FromMilliseconds(100));
    }
    #endif
  }
}
# define DEBUG_BREAK() DebugBreak()
#elif defined(DEBUG_BREAK_ASM)
# define DEBUG_BREAK() DEBUG_BREAK_ASM()
#else
# error "don't know how to debug break on this architecture/OS"
#endif

void Debugger::Break() {
  // NOTE: This code MUST be async-signal safe (it's used by in-process
  // stack dumping signal handler). NO malloc or stdio is allowed here.

  // Linker's ICF feature may merge this function with other functions with the
  // same definition (e.g. any function whose sole job is to call abort()) and
  // it may confuse the crash report processing system.
  static int static_variable_to_make_this_function_unique = 0;
  DebugAlias(&static_variable_to_make_this_function_unique);

  DEBUG_BREAK();
  #if defined(NDEBUG)
  // Terminate the program after signaling the debug break.
  _exit(1);
  #endif
}

} // namespace stp
