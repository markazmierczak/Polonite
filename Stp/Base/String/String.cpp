// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/String/String.h"

#include "Base/Memory/Allocate.h"
#include "Base/Type/Comparable.h"

namespace stp {

/**
 * @fn String::String(StringSpan text)
 * Construct a UTF-8 string.
 * A copy of given |text| is made for new string.
 */
String::String(StringSpan text) {
  if (text.isEmpty()) {
    data_ = "";
    length_ = 0;
    capacity_ = 0;
  } else {
    char* data = static_cast<char*>(allocateMemory(text.length() + 1));
    length_ = text.length();
    capacity_ = text.length();
    uninitializedCopy(data, text.data(), text.length());
    *(data + length_) = '\0';
    data_ = data;
  }
}

String::String(const String& o) {
  if (UNLIKELY(this == &o))
    return;
  if (o.capacity_ > 0 && o.length_ > 0) {
    char* data = static_cast<char*>(allocateMemory(o.length_ + 1));
    uninitializedCopy(data, o.data_, o.length_ + 1);
    data_ = data;
    length_ = o.length_;
    capacity_ = o.length_;
  } else {
    data_ = o.data_;
    length_ = o.length_;
    capacity_ = o.capacity_;
  }
}

void String::assign(StringSpan o) {
  // No need for special case when span points inside this string.
  char* data;
  if (capacity_ < o.length()) {
    if (capacity_ > 0) {
      data = const_cast<char*>(data_);
      data = static_cast<char*>(reallocateMemory(data, o.length() + 1));
    } else {
      data = static_cast<char*>(allocateMemory(o.length() + 1));
    }
    capacity_ = o.length();
  } else {
    data = const_cast<char*>(data_);
  }
  uninitializedCopy(data, o.data(), o.length());
  length_ = o.length();
  *(data + length_) = '\0';
}

/**
 * Do not use this function for string literals.
 * @param cstr Null-terminated string UTF-8 encoded.
 * @return A new string value with copy of given C-string.
 */
String String::fromCString(const char* cstr) {
  int length = getLengthOfCString(cstr);
  if (length == 0)
    return String();

  char* data = static_cast<char*>(allocateMemory(length + 1));
  uninitializedCopy(data, cstr, length + 1);
  return String(data, length, length);
}

String String::createUninitialized(int length, char*& out_data) {
  ASSERT(length >= 0);
  if (length == 0) {
    out_data = nullptr;
    return String();
  }
  char* data = static_cast<char*>(allocateMemory(length + 1));
  *(data + length) = '\0';
  return String(data, length, length);
}

} // namespace stp
