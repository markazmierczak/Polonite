// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/Console.h"

#include "Base/FileSystem/KnownPaths.h"
#include "Base/Io/FileStream.h"
#include "Base/Process/CommandLine.h"
#include "Base/Text/Codec/Utf8Encoding.h"
#include "Base/Text/Utf.h"
#include "Base/Type/Formattable.h"

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
    std_ = openStdStream(std_descriptor);
    if (!std_) {
      active_destinations_ &= ~StandardOutputDestination;
    } else {
      uses_colors_ = shouldUseColors(*std_);
      if (uses_colors_)
        fetchDefaultColors();
    }
  }
}

ConsoleWriter::~ConsoleWriter() {
  flush();
}

void ConsoleWriter::onFlush() {
  // ConsoleWriter buffers output until newline is written.
  AutoLock auto_lock(&lock_);
  if (!buffer_.isEmpty())
    printBuffer(buffer_.size());
}

void ConsoleWriter::printBuffer(int ready_size) {
  *this << buffer_.getSlice(0, ready_size);
  buffer_.removePrefix(ready_size);
}

#if OS(LINUX)
void ConsoleWriter::printToSystemDebugLog(StringSpan text) {
}
#endif

bool ConsoleWriter::isConsoleWriter() const {
  return true;
}

static FilePath resolveLogFilePath(const String* option) {
  String basename;
  if (!option || option->isEmpty())
    basename = "debug.log";
  else
    basename = *option;

  auto path = FilePath::FromString(basename);
  if (!path.IsAbsolute())
    path = CombineFilePaths(GetCurrentDirPath(), path);
  return path;
}

static int determineDestinations(const CommandLine& command_line) {
  #if defined(NDEBUG)
  bool enable_logging = false;
  StringSpan InvertLoggingSwitch = "enable-logging";
  #else
  bool enable_logging = true;
  StringSpan InvertLoggingSwitch = "disable-logging";
  #endif

  if (command_line.Has(InvertLoggingSwitch)) {
    enable_logging = !enable_logging;
  }

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

void Console::classInit() {
  auto& command_line = CommandLine::ForCurrentProcess();
  int active_destinations = determineDestinations(command_line);

  // Initialize std streams first. One of them might be closed and
  // opening LogFile may assign std-err for example.
  if (active_destinations & FileDestination) {
    const String* option = command_line.TryGet(LogToFileSwitch);
    FilePath path = resolveLogFilePath(option);

    // Delete old log file.
    File::Delete(path);

    g_log_file = openLogFile(path);
    if (!g_log_file)
      active_destinations &= ~FileDestination;
  }
  g_out_ = new ConsoleWriter(ConsoleWriter::StdOut, active_destinations);
  g_err_ = new ConsoleWriter(ConsoleWriter::StdErr, active_destinations);
}

void Console::classFini() {
  delete g_out_;
  delete g_err_;
  delete g_log_file;
}

ConsoleWriter* Console::g_out_ = nullptr;
ConsoleWriter* Console::g_err_ = nullptr;

TextEncoding ConsoleWriter::GetEncoding() const {
  return BuiltinTextEncodings::Utf8();
}

void ConsoleWriter::onWriteString(StringSpan text) {
  auto data = BufferSpan(text);

  if (active_destinations_ & StandardOutputDestination)
    std_->Write(data);
  if (active_destinations_ & SystemDebugLogDestination) {
    if (log_level_ != LogLevelUSER)
      printToSystemDebugLog(text);
  }
  if (active_destinations_ & FileDestination)
    g_log_file->Write(data);
}

} // namespace stp
