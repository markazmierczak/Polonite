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

  int PeekAscii();
  int32_t Peek() { return OnPeek(); }

  // Reads the next character and advances the position by one character.
  // Returns the next character from the text reader, or -1 if no more
  // characters are available (or character cannot be encoded in ASCII).
  int ReadAscii();
  int32_t Read() { return OnRead(); }

  // Reads a specified maximum number of characters and writes the data to a buffer.
  // Returns the number of characters that have been read.
  int ReadAscii(char* data, int count);
  int Read(char* data, int count) { return OnRead(data, count); }

  bool ReadLineAscii(String& output);
  bool ReadLine(String& output);

 protected:
  virtual int32_t OnRead() = 0;
  virtual int32_t OnPeek() = 0;

  virtual int OnRead(char* dst, int count) = 0;
};

} // namespace stp

#endif // STP_BASE_IO_TEXTREADER_H_
