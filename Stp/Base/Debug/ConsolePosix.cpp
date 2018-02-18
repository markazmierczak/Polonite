// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/Console.h"

#include "Base/Containers/Array.h"
#include "Base/Io/FileStream.h"
#include "Base/Memory/OwnPtr.h"
#include "Base/System/Environment.h"

#include <fcntl.h>

namespace stp {

typedef char AnsiColorSequence[7];

struct AnsiColor {
  char code;
  bool intense;
};

bool ConsoleWriter::shouldUseColors(const FileStream& stream) {
  int fd = stream.GetNativeFile();

  if (!::isatty(fd))
    return false;

  // Keep ordinary sorted.
  constexpr Array<StringSpan, 9> SupportedTerminals = {
    "cygwin",
    "linux",
    "rxvt-unicode",
    "rxvt-unicode-256color",
    "screen",
    "screen-256color",
    "xterm",
    "xterm-256color",
    "xterm-color",
  };
  String term_name;
  if (!Environment::TryGet("TERM", term_name))
    return false;
  return SupportedTerminals.Contains(term_name);
}

static AnsiColor getAnsiColor(ConsoleColor color) {
  AnsiColor ansi_color;

  const ConsoleColor FirstIntense = ConsoleColor::FirstIntense;
  if (color >= FirstIntense) {
    color = static_cast<ConsoleColor>(static_cast<int>(color) - static_cast<int>(FirstIntense));
    ansi_color.intense = true;
  } else {
    ansi_color.intense = false;
  }

  ansi_color.code = static_cast<int>(color) + '0';
  ASSERT('0' <= ansi_color.code && ansi_color.code <= '7');
  return ansi_color;
}

void ConsoleWriter::setForegroundColor(ConsoleColor color) {
  if (!uses_colors_)
    return;
  // TODO no need to flush early, just set destinations temporarily
  Flush();

  AnsiColor ansi = getAnsiColor(color);
  byte_t sequence[] = "\033[;??m";
  sequence[3] = ansi.intense ? '9' : '3';
  sequence[4] = ansi.code;
  std_->Write(BufferSpan(sequence));
}

void ConsoleWriter::setBackgroundColor(ConsoleColor color) {
  if (!uses_colors_)
    return;
  Flush();

  AnsiColor ansi = getAnsiColor(color);
  if (ansi.intense) {
    byte_t sequence[] = "\033[;10?m";
    sequence[5] = ansi.code;
    std_->Write(BufferSpan(sequence));
  } else {
    byte_t sequence[] = "\033[;4?m";
    sequence[4] = ansi.code;
    std_->Write(BufferSpan(sequence));
  }
}

void ConsoleWriter::setColors(ConsoleColor foreground, ConsoleColor background) {
  setForegroundColor(foreground);
  setBackgroundColor(background);
}

void ConsoleWriter::fetchDefaultColors() {}

void ConsoleWriter::resetColors() {
  if (!uses_colors_)
    return;
  Flush();
  byte_t sequence[] = "\033[m";
  std_->Write(BufferSpan(sequence));
}

FileStream* ConsoleWriter::openStdStream(StdDescriptor std_descriptor) {
  int fd = std_descriptor;
  // Check if descriptor is valid.
  if (::fcntl(fd, F_GETFD) == -1)
    return nullptr;

  auto* stream = new FileStream();
  stream->OpenNative(fd, FileAccess::WriteOnly, FileStream::DontClose);
  return stream;
}

FileStream* Console::openLogFile(const FilePath& path) {
  auto stream = OwnPtr<FileStream>::New();
  if (!IsOk(stream->TryCreate(path, FileMode::Create, FileAccess::WriteOnly)))
    return nullptr;

  return stream.release();
}

} // namespace stp
