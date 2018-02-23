// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_TEXTREADER_H_
#define STP_BASE_IO_TEXTREADER_H_

#include "Base/Containers/List.h"

namespace stp {

class BASE_EXPORT TextReader {
 public:
  TextReader() {}
  virtual ~TextReader() {}

  int peekAscii();
  int32_t peek() { return onPeek(); }

  // Reads the next character and advances the position by one character.
  // Returns the next character from the text reader, or -1 if no more
  // characters are available (or character cannot be encoded in ASCII).
  int readAscii();
  int32_t read() { return onRead(); }

  // Reads a specified maximum number of characters and writes the data to a buffer.
  // Returns the number of characters that have been read.
  int readAscii(char* data, int count);
  int read(char* data, int count) { return OnRead(data, count); }

  bool readLineAscii(String& output);
  bool readLine(String& output);

 protected:
  virtual int32_t onRead() = 0;
  virtual int32_t onPeek() = 0;

  virtual int OnRead(char* dst, int count) = 0;
};

} // namespace stp

#endif // STP_BASE_IO_TEXTREADER_H_
