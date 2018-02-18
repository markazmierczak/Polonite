// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Debug/StackTrace.h"

#include "Base/App/Application.h"
#include "Base/Linux/ProcMaps.h"

#include <android/log.h>
#include <unwind.h>

#ifdef __LP64__
#define FMT_ADDR  "0x%016lx"
#else
#define FMT_ADDR  "0x%08x"
#endif

namespace {

struct StackCrawlState {
  StackCrawlState(uintptr_t* frames, int max_depth)
      : frames(frames), max_depth(max_depth) {}

  uintptr_t* frames;
  int frame_count = 0;
  int max_depth;
  bool have_skipped_self = false;
};

_Unwind_Reason_Code TraceStackFrame(_Unwind_Context* context, void* arg) {
  StackCrawlState* state = static_cast<StackCrawlState*>(arg);
  uintptr_t ip = _Unwind_GetIP(context);

  // The first stack frame is this function itself.  Skip it.
  if (ip != 0 && !state->have_skipped_self) {
    state->have_skipped_self = true;
    return _URC_NO_REASON;
  }

  state->frames[state->frame_count++] = ip;
  if (state->frame_count >= state->max_depth)
    return _URC_END_OF_STACK;
  return _URC_NO_REASON;
}

} // namespace

namespace stp {

bool StackTrace::EnableInProcessDump() {
  // When running in an application, our code typically expects SIGPIPE
  // to be ignored.  Therefore, when testing that same code, it should run
  // with SIGPIPE ignored as well.
  // TODO(phajdan.jr): De-duplicate this SIGPIPE code.
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = SIG_IGN;
  sigemptyset(&action.sa_mask);
  return (sigaction(SIGPIPE, &action, NULL) == 0);
}

StackTrace::StackTrace() {
  StackCrawlState state(reinterpret_cast<uintptr_t*>(trace_), MaxTraces);
  _Unwind_Backtrace(&TraceStackFrame, &state);
  count_ = state.frame_count;
}

// NOTE: Native libraries in APKs are stripped before installing. Print out the
// relocatable address and library names so host computers can use tools to
// symbolize and demangle (e.g., addr2line, c++filt).
void StackTrace::OutputToStream(std::ostream* os) const {
  std::string proc_maps;
  std::vector<MappedMemoryRegion> regions;
  // Allow IO to read /proc/self/maps. Reading this file doesn't hit the disk
  // since it lives in procfs, and this is currently used to print a stack trace
  // on fatal log messages in debug builds only. If the restriction is enabled
  // then it will recursively trigger fatal failures when this enters on the
  // UI thread.
  const char* app_name = Application::instance().getName().c_str();
  if (!ReadProcMaps(&proc_maps)) {
    __android_log_write(
        ANDROID_LOG_ERROR, app_name, "Failed to read /proc/self/maps");
  } else if (!ParseProcMaps(proc_maps, &regions)) {
    __android_log_write(
        ANDROID_LOG_ERROR, app_name, "Failed to parse /proc/self/maps");
  }

  for (int i = 0; i < count_; ++i) {
    // Subtract one as return address of function may be in the next
    // function when a function is annotated as noreturn.
    uintptr_t address = reinterpret_cast<uintptr_t>(trace_[i]) - 1;

    std::vector<MappedMemoryRegion>::iterator iter = regions.begin();
    while (iter != regions.end()) {
      if (address >= iter->start && address < iter->end &&
          !iter->path.empty()) {
        break;
      }
      ++iter;
    }

    *os << StringPrintf("#%02zd " FMT_ADDR " ", i, address);

    if (iter != regions.end()) {
      uintptr_t rel_pc = address - iter->start + iter->offset;
      const char* path = iter->path.c_str();
      *os << StringPrintf("%s+" FMT_ADDR, path, rel_pc);
    } else {
      *os << "<unknown>";
    }

    *os << "\n";
  }
}

} // namespace stp
