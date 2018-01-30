// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/Console.h"

#include "Base/FileSystem/KnownPaths.h"
#include "Base/Io/FileStream.h"
#include "Base/Process/CommandLine.h"
#include "Base/Text/TextEncoding.h"

namespace stp {

// Where to record console output? A flat file and/or system debug log.
enum ConsoleDestination {
  StandardOutputDestination = 1 << 0,
  SystemDebugLogDestination = 1 << 1,
  FileDestination           = 1 << 2,
};

constexpr StringSpan LogToStdSwitch = "log-to-std";
constexpr StringSpan LogToFileSwitch = "log-to-file";

static FileStream* g_log_file = nullptr;

ConsoleWriter::ConsoleWriter(StdDescriptor std_descriptor, int active_destinations)
    : active_destinations_(active_destinations) {
  if (active_destinations_ & StandardOutputDestination) {
    std_ = OpenStdStream(std_descriptor);
    if (!std_) {
      active_destinations_ &= ~StandardOutputDestination;
    } else {
      uses_colors_ = ShouldUseColors(*std_);
      if (uses_colors_)
        FetchDefaultColors();
    }
  }
}

ConsoleWriter::~ConsoleWriter() {
  Flush();
}

void ConsoleWriter::OnFlush() {
  // ConsoleWriter buffers output until newline is written.
  AutoLock auto_lock(&lock_);
  if (!buffer_.IsEmpty())
    PrintBuffer(buffer_.size());
}

void ConsoleWriter::PrintBuffer(int ready_size) {
  Print(buffer_.GetSlice(0, ready_size));
  buffer_.RemovePrefix(ready_size);
}

void ConsoleWriter::Print(StringSpan text) {
  auto data = BufferSpan(text);

  if (active_destinations_ & StandardOutputDestination)
    std_->Write(data);
  if (active_destinations_ & SystemDebugLogDestination) {
    if (log_level_ != LogLevelUSER)
      PrintToSystemDebugLog(text);
  }
  if (active_destinations_ & FileDestination)
    g_log_file->Write(data);
}

#if OS(LINUX)
void ConsoleWriter::PrintToSystemDebugLog(StringSpan text) {
}
#endif

bool ConsoleWriter::IsConsoleWriter() const {
  return true;
}

static FilePath ResolveLogFilePath(const String* option) {
  String basename;
  if (!option || option->IsEmpty())
    basename = "debug.log";
  else
    basename = *option;

  auto path = FilePath::FromString(basename);
  if (!path.IsAbsolute())
    path = CombineFilePaths(GetCurrentDirPath(), path);
  return path;
}

static int DetermineDestinations(const CommandLine& command_line) {
  #if defined(NDEBUG)
  bool enable_logging = false;
  StringSpan InvertLoggingSwitch = "enable-logging";
  #else
  bool enable_logging = true;
  StringSpan InvertLoggingSwitch = "disable-logging";
  #endif

  if (command_line.Has(InvertLoggingSwitch))
    enable_logging = !enable_logging;

  #if defined(NDEBUG)
  int DefaultDestinations = FileDestination;
  #else
  int DefaultDestinations = SystemDebugLogDestination | StandardOutputDestination;
  #endif

  int destinations = 0;
  if (enable_logging) {
    destinations = DefaultDestinations;

    if (command_line.Has(LogToStdSwitch))
      destinations |= StandardOutputDestination;

    if (command_line.Has(LogToFileSwitch))
      destinations |= FileDestination;
  }
  return destinations;
}

void Console::ClassInit() {
  auto& command_line = CommandLine::ForCurrentProcess();
  int active_destinations = DetermineDestinations(command_line);

  // Initialize std streams first. One of them might be closed and
  // opening LogFile may assign std-err for example.
  if (active_destinations & FileDestination) {
    const String* option = command_line.TryGet(LogToFileSwitch);
    FilePath path = ResolveLogFilePath(option);

    // Delete old log file.
    File::Delete(path);

    g_log_file = OpenLogFile(path);
    if (!g_log_file)
      active_destinations &= ~FileDestination;
  }
  g_out_ = new ConsoleWriter(ConsoleWriter::StdOut, active_destinations);
  g_err_ = new ConsoleWriter(ConsoleWriter::StdErr, active_destinations);
}

void Console::ClassFini() {
  delete g_out_;
  delete g_err_;
  delete g_log_file;
}

ConsoleWriter* Console::g_out_ = nullptr;
ConsoleWriter* Console::g_err_ = nullptr;

TextEncoding ConsoleWriter::GetEncoding() const {
  return BuiltinTextEncoding::Utf8();
}

void ConsoleWriter::OnWriteAsciiChar(char c) {
  OnWriteAscii(StringSpan(&c, 1));
}

void ConsoleWriter::OnWriteUnicodeChar(char32_t c) {
  char buffer[Utf8::MaxEncodedCodepointLength];
  int n = EncodeUtf(buffer, c);
  OnWriteUtf8(StringSpan(buffer, n));
}

void ConsoleWriter::OnWriteAscii(StringSpan text) {
  Print(text);
}

void ConsoleWriter::OnWriteUtf8(StringSpan text) {
  Print(text);
}

void ConsoleWriter::OnWriteUtf16(String16Span text) {
  OnWriteUtf8(ToString(text));
}

void ConsoleWriter::OnWriteEncoded(const BufferSpan& text, TextEncoding encoding) {
  throw NotImplementedException();
}

} // namespace stp
