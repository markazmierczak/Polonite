// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Debug/StackTrace.h"

#include "Base/Debug/Log.h"
#include "Base/FileSystem/KnownPaths.h"
#include "Base/Memory/OwnPtr.h"
#include "Base/Text/FormatMany.h"
#include "Base/Thread/Lock.h"

#include <windows.h>
#include <dbghelp.h>

namespace stp {

namespace {

// Previous unhandled filter. Will be called if not NULL when we intercept an
// exception. Only used in unit tests.
LPTOP_LEVEL_EXCEPTION_FILTER g_previous_unhandled_exception_filter = nullptr;

int g_symbols_initialized = -1;

static BasicLock g_symbolizer_lock = BASIC_LOCK_INITIALIZER;

// Prints the exception call stack.
// This is the unit tests exception filter.
long WINAPI StackDumpExceptionFilter(EXCEPTION_POINTERS* info) {
  StackTrace(info).PrintToConsole();
  if (g_previous_unhandled_exception_filter)
    return g_previous_unhandled_exception_filter(info);
  return EXCEPTION_CONTINUE_SEARCH;
}

FilePath GetExePath() {
  wchar_t system_buffer[MAX_PATH];
  GetModuleFileName(NULL, system_buffer, MAX_PATH);
  system_buffer[MAX_PATH - 1] = L'\0';
  return FilePath::FromNullTerminated(system_buffer);
}

bool InitializeSymbols() {
  if (g_symbols_initialized >= 0)
    return g_symbols_initialized > 0;
  g_symbols_initialized = 0;

  HANDLE current_process_handle = GetCurrentProcess();

  // Defer symbol load until they're needed, use undecorated names, and get line numbers.
  SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);

  if (!SymInitialize(current_process_handle, NULL, TRUE)) {
    // TODO(awong): Handle error: SymInitialize can fail with ERROR_INVALID_PARAMETER.
    // When it fails, we should not call debugbreak since it kills the current
    // process (prevents future tests from running or kills the browser
    // process).
    LOG(ERROR, "SymInitialize failed");
    return false;
  }

  // When transferring the binaries e.g. between bots, path put
  // into the executable will get off. To still retrieve symbols correctly,
  // add the directory of the executable to symbol search path.
  // All following errors are non-fatal.
  const int SymbolsArraySize = 1024;
  auto symbols_path = OwnPtr<wchar_t[]>::New(SymbolsArraySize);

  // Note: The below function takes buffer size as number of characters,
  // not number of bytes!
  if (!::SymGetSearchPathW(current_process_handle, symbols_path.get(), SymbolsArraySize)) {
    LOG(WARN, "SymGetSearchPath failed");
    return false;
  }

  FilePath path = GetExecutableDirPath();
  WString new_path = WString::ConcatArgs(
      MakeSpanFromNullTerminated(symbols_path.get()),
      L";",
      GetExePath().GetDirectoryName().AsCharactersUnsafe());

  if (!::SymSetSearchPathW(current_process_handle, ToNullTerminated(new_path))) {
    LOG(WARN, "SymSetSearchPath failed");
    return false;
  }
  g_symbols_initialized = 1;
  return true;
}

} // namespace

union SymbolInfoUnion {
  static constexpr unsigned MaxNameLength = MAX_SYM_NAME;
  static constexpr unsigned StructSize = sizeof(SYMBOL_INFOW);
  static constexpr unsigned Size = StructSize + MaxNameLength * sizeof(wchar_t);

  // Zero initialize basic information.
  SymbolInfoUnion() : info() {
    info.SizeOfStruct = StructSize;
    info.MaxNameLen = MaxNameLength;
  }

  SYMBOL_INFOW info;
  char bytes[Size];
};

void FormatSymbol(TextWriter& out, void* pc) {
  HANDLE current_process_handle = GetCurrentProcess();

  AutoLock auto_lock(&g_symbolizer_lock);

  DWORD_PTR frame = reinterpret_cast<DWORD_PTR>(pc);
  SymbolInfoUnion symbol;
  DWORD64 sym_displacement = 0;
  BOOL has_symbol = ::SymFromAddrW(current_process_handle, frame, &sym_displacement, &symbol.info);

  // Attempt to retrieve line number information.
  DWORD line_displacement = 0;
  IMAGEHLP_LINEW64 line = {};
  line.SizeOfStruct = sizeof(IMAGEHLP_LINEW64);
  BOOL has_line = ::SymGetLineFromAddrW64(current_process_handle, frame, &line_displacement, &line);

  if (has_symbol) {
    out << MakeSpanFromNullTerminated(symbol.info.Name);
    out << '+' << sym_displacement;
  } else {
    out << "(no symbol)";
  }
  if (has_line) {
    out << " (" << MakeSpanFromNullTerminated(line.FileName) << ':' << line.LineNumber << ")";
  }
}

bool StackTrace::EnableInProcessDump() {
  // Add stack dumping support on exception on windows.
  g_previous_unhandled_exception_filter = SetUnhandledExceptionFilter(&StackDumpExceptionFilter);

  // Need to initialize symbols early in the process or else this fails on
  // swarming (since symbols are in different directory than in the exes) and
  // also release x64.
  return InitializeSymbols();
}

// Disable optimizations for the StackTrace::StackTrace function. It is
// important to disable at least frame pointer optimization ("y"), since
// that breaks CaptureStackBackTrace() and prevents StackTrace from working
// in Release builds (it may still be janky if other frames are using FPO,
// but at least it will make it further).
#if COMPILER(MSVC)
#pragma optimize("", off)
#endif

StackTrace::StackTrace() {
  // When walking our own stack, use CaptureStackBackTrace().
  count_ = CaptureStackBackTrace(0, isizeofArray(trace_), trace_, NULL);
}

#if COMPILER(MSVC)
#pragma optimize("", on)
#endif

StackTrace::StackTrace(EXCEPTION_POINTERS* exception_pointers) {
  InitTrace(exception_pointers->ContextRecord);
}

StackTrace::StackTrace(const CONTEXT* context) {
  InitTrace(context);
}

void StackTrace::InitTrace(const CONTEXT* context_record) {
  // StackWalk64 modifies the register context in place, so we have to copy it
  // so that downstream exception handlers get the right context.  The incoming
  // context may have had more register state (YMM, etc) than we need to unwind
  // the stack. Typically StackWalk64 only needs integer and control registers.
  CONTEXT context_copy;
  memcpy(&context_copy, context_record, sizeof(context_copy));
  context_copy.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;

  // When walking an exception stack, we need to use StackWalk64().
  count_ = 0;
  // Initialize stack walking.
  STACKFRAME64 stack_frame;
  memset(&stack_frame, 0, sizeof(stack_frame));

  #if defined(_WIN64)
  int machine_type = IMAGE_FILE_MACHINE_AMD64;
  stack_frame.AddrPC.Offset = context_record->Rip;
  stack_frame.AddrFrame.Offset = context_record->Rbp;
  stack_frame.AddrStack.Offset = context_record->Rsp;
  #else
  int machine_type = IMAGE_FILE_MACHINE_I386;
  stack_frame.AddrPC.Offset = context_record->Eip;
  stack_frame.AddrFrame.Offset = context_record->Ebp;
  stack_frame.AddrStack.Offset = context_record->Esp;
  #endif

  stack_frame.AddrPC.Mode = AddrModeFlat;
  stack_frame.AddrFrame.Mode = AddrModeFlat;
  stack_frame.AddrStack.Mode = AddrModeFlat;
  while (StackWalk64(machine_type,
                     GetCurrentProcess(),
                     GetCurrentThread(),
                     &stack_frame,
                     &context_copy,
                     NULL,
                     &SymFunctionTableAccess64,
                     &SymGetModuleBase64,
                     NULL) &&
         count_ < MaxTraces) {
    trace_[count_++] = reinterpret_cast<void*>(stack_frame.AddrPC.Offset);
  }

  for (int i = count_; i < MaxTraces; ++i)
    trace_[i] = nullptr;
}

} // namespace stp
