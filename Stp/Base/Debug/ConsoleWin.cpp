// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/Console.h"

#include "Base/Io/FileStream.h"
#include "Base/Text/FormatMany.h"

#include <io.h>
#include <stdio.h>
#include <windows.h>

namespace stp {

// Converts ConsoleColor to Win32 bitmask.
static unsigned getColorAttribute(ConsoleColor color) {

  const ConsoleColor FirstIntense = ConsoleColor::FirstIntense;
  unsigned intensity = 0;
  if (color >= FirstIntense) {
    intensity = FOREGROUND_INTENSITY;
    color = static_cast<ConsoleColor>(static_cast<int>(color) - static_cast<int>(FirstIntense));
  }

  unsigned output = 0;
  switch (color) {
    case ConsoleColor::Black:
      output = 0;
      break;
    case ConsoleColor::DarkBlue:
      output = FOREGROUND_BLUE;
      break;
    case ConsoleColor::DarkGreen:
      output = FOREGROUND_GREEN;
      break;
    case ConsoleColor::DarkRed:
      output = FOREGROUND_RED;
      break;

    case ConsoleColor::DarkCyan:
      output = FOREGROUND_GREEN | FOREGROUND_BLUE;
      break;
    case ConsoleColor::DarkMagenta:
      output = FOREGROUND_RED | FOREGROUND_BLUE;
      break;
    case ConsoleColor::DarkYellow:
      output = FOREGROUND_RED | FOREGROUND_GREEN;
      break;
    case ConsoleColor::DarkGray:
      output = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
      break;

    default:
      UNREACHABLE();
  }
  return output | intensity;
}

void ConsoleWriter::setForegroundColor(ConsoleColor color) {
  unsigned foreground = getColorAttribute(color);
  unsigned attributes = (std_attributes_ & ~0x0F) | (foreground << 0);
  updateAttributes(attributes);
}

void ConsoleWriter::setBackgroundColor(ConsoleColor color) {
  unsigned background = getColorAttribute(color);
  unsigned attributes = (std_attributes_ & ~0xF0) | (background << 4);
  updateAttributes(attributes);
}

void ConsoleWriter::setColors(ConsoleColor foreground_, ConsoleColor background_) {
  unsigned foreground = getColorAttribute(foreground_);
  unsigned background = getColorAttribute(background_);
  unsigned attributes = (std_attributes_ & ~0xFF) | (background << 4) | foreground;
  updateAttributes(attributes);
}

void ConsoleWriter::fetchDefaultColors() {
  CONSOLE_SCREEN_BUFFER_INFO buffer_info;
  if (!GetConsoleScreenBufferInfo(std_->GetNativeFile(), &buffer_info)) {
    uses_colors_ = false;
    return;
  }
  default_std_attributes_ = buffer_info.wAttributes;
  std_attributes_ = default_std_attributes_;
}

void ConsoleWriter::resetColors() {
  updateAttributes(default_std_attributes_);
}

void ConsoleWriter::UpdateAttributes(unsigned attributes) {
  if (!uses_colors_)
    return;
  if (std_attributes_ == attributes)
    return;
  std_attributes_ = attributes;

  flush();

  ::SetConsoleTextAttribute(std_->GetNativeFile(), attributes);
}

void ConsoleWriter::printToSystemDebugLog(StringSpan text) {
  ::OutputDebugStringW(ToNullTerminated(ToWString(text)));
}

bool ConsoleWriter::shouldUseColors(const FileStream& stream) {
  HANDLE std_handle = stream.GetNativeFile();
  if (::GetFileType(std_handle) != FILE_TYPE_CHAR)
    return false;

  // Test whether std_handle represents TTY through GetConsoleMode.
  DWORD mode;
  return ::GetConsoleMode(std_handle, &mode) != 0;
}

FileStream* ConsoleWriter::openStdStream(StdDescriptor std_descriptor) {
  // Convert std_descriptor to index for Win32 API.
  DWORD handle_index = [](StdDescriptor desc) {
    switch(desc) {
      case StdIn:
        return STD_INPUT_HANDLE;
      case StdOut:
        return STD_OUTPUT_HANDLE;
      case StdErr:
        return STD_ERROR_HANDLE;
    }
    UNREACHABLE(return 0ul);
  } (std_descriptor);

  HANDLE handle = ::GetStdHandle(handle_index);
  if (handle == INVALID_HANDLE_VALUE)
    return nullptr;

  auto* stream = new FileStream();
  auto access = std_descriptor == StdIn ? FileAccess::ReadOnly : FileAccess::WriteOnly;
  stream->OpenNative(handle, access, FileStream::DontClose);
  return stream;
}

FileStream* Console::openLogFile(const FilePath& path) {
  // The FILE_APPEND_DATA access mask ensures that the file is atomically
  // appended to across accesses from multiple threads.
  // https://msdn.microsoft.com/en-us/library/windows/desktop/aa364399(v=vs.85).aspx
  // https://msdn.microsoft.com/en-us/library/windows/desktop/aa363858(v=vs.85).aspx
  HANDLE log_file = ::CreateFileW(
      ToNullTerminated(path), FILE_APPEND_DATA,
      FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
      OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (log_file == INVALID_HANDLE_VALUE)
    return nullptr;

  auto* stream = new FileStream();
  stream->OpenNative(log_file);
  return stream;
}

void RouteStdioToConsole(bool create_console_if_not_found) {
  // Don't change anything if stdout or stderr already point to a
  // valid stream.
  //
  // We don't use GetStdHandle() to check stdout/stderr here because
  // it can return dangling IDs of handles that were never inherited
  // by this process.  These IDs could have been reused by the time
  // this function is called.  The CRT checks the validity of
  // stdout/stderr on startup (before the handle IDs can be reused).
  // _fileno(stdout) will return -2 (_NO_CONSOLE_FILENO) if stdout was
  // invalid.
  if (_fileno(stdout) >= 0 || _fileno(stderr) >= 0) {
    // _fileno was broken for SUBSYSTEM:WINDOWS from VS2010 to VS2012/2013.
    // Confirm that the underlying HANDLE is valid before aborting.

    intptr_t stdout_handle = _get_osfhandle(_fileno(stdout));
    intptr_t stderr_handle = _get_osfhandle(_fileno(stderr));
    if (stdout_handle >= 0 || stderr_handle >= 0)
      return;
  }

  if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
    unsigned int result = GetLastError();
    // Was probably already attached.
    if (result == ERROR_ACCESS_DENIED)
      return;
    // Don't bother creating a new console for each child process if the
    // parent process is invalid (eg: crashed).
    if (result == ERROR_GEN_FAILURE)
      return;
    if (!create_console_if_not_found)
      return;
    // Make a new console if attaching to parent fails with any other error.
    // It should be ERROR_INVALID_HANDLE at this point, which means the
    // browser was likely not started from a console.
    AllocConsole();
  }

  // Arbitrary byte count to use when buffering output lines.  More
  // means potential waste, less means more risk of interleaved
  // log-lines in output.
  constexpr int OutputBufferSize = 64 * 1024;

  if (freopen("CONOUT$", "w", stdout)) {
    setvbuf(stdout, nullptr, _IOLBF, OutputBufferSize);
    // Overwrite FD 1 for the benefit of any code that uses this FD
    // directly.  This is safe because the CRT allocates FDs 0, 1 and
    // 2 at startup even if they don't have valid underlying Windows
    // handles.  This means we won't be overwriting an FD created by
    // _open() after startup.
    _dup2(_fileno(stdout), 1);
  }
  if (freopen("CONOUT$", "w", stderr)) {
    setvbuf(stderr, nullptr, _IOLBF, OutputBufferSize);
    _dup2(_fileno(stderr), 2);
  }
}

} // namespace stp
