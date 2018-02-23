// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/TextReader.h"

#include "Base/Text/Utf.h"

namespace stp {

int TextReader::peekAscii() {
  int32_t ch = peek();
  if (0 <= ch && ch <= 0x7F)
    return ch;
  return -1;
}

int TextReader::readAscii() {
  int32_t ch = read();
  if (0 <= ch && ch <= 0x7F)
    return ch;
  return -1;
}

int TextReader::readAscii(char* data, int count) {
  int n = OnRead(data, count);
  if (n > 0) {
    if (!StringSpan(data, n).isAscii())
      return -1;
  }
  return n;
}

bool TextReader::readLineAscii(String& out) {
  out.clear();

  while (true) {
    int ch = readAscii();
    if (ch < 0)
      break;

    if (ch == '\n' || ch == '\r') {
      if (ch == '\r' && peekAscii() == '\n')
        readAscii();
      return true;
    }
    out.append(static_cast<char>(ch));
  }
  return !out.isEmpty();
}

bool TextReader::readLine(String& out) {
  out.clear();

  while (true) {
    int32_t ch = read();
    if (ch < 0)
      break;

    if (ch == '\n' || ch == '\r') {
      if (ch == '\r' && peek() == '\n')
        read();
      return true;
    }
    int encoded_length = Utf8::EncodedLength(ch);
    char* encoded_ch = out.appendUninitialized(encoded_length);
    int result = Utf8::Encode(encoded_ch, ch);
    ASSERT_UNUSED(result == encoded_length, result);
  }
  return !out.isEmpty();
}

} // namespace stp
