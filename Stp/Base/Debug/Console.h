// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_DEBUG_CONSOLE_H_
#define STP_BASE_DEBUG_CONSOLE_H_

#include "Base/Containers/List.h"
#include "Base/Debug/Log.h"
#include "Base/Io/TextWriter.h"
#include "Base/Thread/Lock.h"

namespace stp {

class FilePath;
class FileStream;

enum class ConsoleColor {
  // NOTE: The order matters for implementation.
  Black,
  DarkRed, DarkGreen, DarkYellow, DarkBlue, DarkMagenta, DarkCyan,
  DarkGray,
  Gray,
  Red, Green, Yellow, Blue, Magenta, Cyan,
  White,

  FirstIntense = Gray,
};

class ConsoleWriter : public TextWriter {
 public:
  enum StdDescriptor { StdIn, StdOut, StdErr };

  ConsoleWriter(StdDescriptor std_descriptor, int active_destinations);
  ~ConsoleWriter() override;

  void SetBackgroundColor(ConsoleColor color);
  void SetForegroundColor(ConsoleColor color);
  void SetColors(ConsoleColor foreground, ConsoleColor background);
  void ResetColors();

  ALWAYS_INLINE void SetLogLevel(LogLevel level) { log_level_ = level; }

  TextEncoding GetEncoding() const override;
  bool IsConsoleWriter() const override;

 private:
  void Print(StringSpan text);
  void PrintBuffer(int ready_size);
  void PrintToSystemDebugLog(StringSpan text);

  void FetchDefaultColors();

  void OnWriteAsciiChar(char c) override;
  void OnWriteUnicodeChar(char32_t c) override;
  void OnWriteAscii(StringSpan text) override;
  void OnWriteUtf8(StringSpan text) override;
  void OnWriteUtf16(String16Span text) override;
  void OnWriteEncoded(const BufferSpan& text, TextEncoding encoding) override;
  void OnFlush() override;

  String buffer_;
  Lock lock_;
  FileStream* std_ = nullptr;
  LogLevel log_level_ = LogLevelUSER;
  int active_destinations_;
  bool uses_colors_ = false;

  #if OS(WIN)
  unsigned std_attributes_ = 0;
  unsigned default_std_attributes_ = 0;
  void UpdateAttributes(unsigned attributes);
  #endif

  static bool ShouldUseColors(const FileStream& stream);

  // The caller becomes owner of the returned stream.
  static FileStream* OpenStdStream(StdDescriptor descriptor);
};

class BASE_EXPORT Console {
  STATIC_ONLY(Console);
 public:
  static ConsoleWriter& Out() { return *g_out_; }
  static ConsoleWriter& Err() { return *g_err_; }

  static void ClassInit();
  static void ClassFini();

 private:
  static ConsoleWriter* g_out_;
  static ConsoleWriter* g_err_;

  static FileStream* OpenLogFile(const FilePath& path);
};

#if OS(WIN)
// Output multi-process printf to the cmd.exe console.
// This is not thread-safe: only call from main thread.
BASE_EXPORT void RouteStdioToConsole(bool create_console_if_not_found);
#endif

} // namespace stp

#endif // STP_BASE_DEBUG_CONSOLE_H_
