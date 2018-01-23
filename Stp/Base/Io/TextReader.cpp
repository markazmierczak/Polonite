// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/TextReader.h"

#include "Base/Text/Utf.h"

namespace stp {

int TextReader::PeekAscii() {
  int32_t ch = Peek();
  if (0 <= ch && ch <= 0x7F)
    return ch;
  return -1;
}

int TextReader::ReadAscii() {
  int32_t ch = Read();
  if (0 <= ch && ch <= 0x7F)
    return ch;
  return -1;
}

int TextReader::ReadAscii(char* data, int count) {
  int n = OnRead(data, count);
  if (n > 0) {
    if (!StringSpan(data, n).IsAscii())
      return -1;
  }
  return n;
}

bool TextReader::ReadLineAscii(String& out) {
  out.Clear();

  while (true) {
    int ch = ReadAscii();
    if (ch < 0)
      break;

    if (ch == '\n' || ch == '\r') {
      if (ch == '\r' && PeekAscii() == '\n')
        ReadAscii();
      return true;
    }
    out.Append(static_cast<char>(ch));
  }
  return !out.IsEmpty();
}

bool TextReader::ReadLine(String& out) {
  out.Clear();

  while (true) {
    int32_t ch = Read();
    if (ch < 0)
      break;

    if (ch == '\n' || ch == '\r') {
      if (ch == '\r' && Peek() == '\n')
        Read();
      return true;
    }
    int encoded_length = Utf8::EncodedLength(ch);
    char* encoded_ch = out.AppendUninitialized(encoded_length);
    int result = Utf8::Encode(encoded_ch, ch);
    ASSERT_UNUSED(result == encoded_length, result);
  }
  return !out.IsEmpty();
}

} // namespace stp
