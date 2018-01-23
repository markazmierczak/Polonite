// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/StringReader.h"

#include "Base/Text/Utf.h"

namespace stp {

int32_t StringReader::OnPeek() {
  return OnReadInternal(false);
}

int32_t StringReader::OnRead() {
  return OnReadInternal(true);
}

inline int32_t StringReader::OnReadInternal(bool advance) {
  int size = string_.size();
  if (size == 0)
    return -1;

  const char* it = string_.data();
  char32_t uc = Utf8::Decode(it, it + size);
  if (Utf8::IsDecodeError(uc))
    return -1;

  if (advance)
    string_.RemovePrefix(it - string_.data());

  return static_cast<int32_t>(uc);
}

int StringReader::OnRead(char* dst, int count) {
  ASSERT(count >= 0);

  if (count > string_.size())
    count = string_.size();

  if (count > 0) {
    UninitializedCopy(dst, string_.data(), count);
    string_.RemovePrefix(count);
  }
  return count;
}

} // namespace stp
