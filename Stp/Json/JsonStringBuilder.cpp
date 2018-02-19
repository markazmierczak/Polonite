// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Json/JsonStringBuilder.h"

#include "Base/Containers/ArrayOps.h"
#include "Base/Memory/Allocate.h"
#include "Base/Text/Utf.h"

namespace stp {

JsonStringBuilder::JsonStringBuilder(const JsonStringBuilder& other) {
  capacity_ = 0;
  AppendString(other.toSpan());
}

JsonStringBuilder& JsonStringBuilder::operator=(const JsonStringBuilder& other) {
  if (this == &other)
    return *this;

  return operator=(other.toSpan());
}

JsonStringBuilder::JsonStringBuilder(JsonStringBuilder&& other) {
  swap(*this, other);
}

JsonStringBuilder& JsonStringBuilder::operator=(JsonStringBuilder&& other) {
  swap(*this, other);
  return *this;
}

JsonStringBuilder& JsonStringBuilder::operator=(StringSpan other) {
  size_ = 0;
  if (capacity_ < 0) {
    data_ = nullptr;
    capacity_ = 0;
  }
  AppendString(other);
  return *this;
}

void JsonStringBuilder::Append(char c) {
  if (capacity_ < 0) {
    ASSERT(data_[size_] == c);
    ++size_;
  } else {
    char* dst = AppendUninitialized(1);
    *dst = c;
  }
}

void JsonStringBuilder::AppendString(StringSpan str) {
  ASSERT(OwnsData());
  if (!str.IsEmpty()) {
    char* dst = AppendUninitialized(str.size());
    uninitializedCopy(dst, str.data(), str.size());
  }
}

void JsonStringBuilder::AppendInPlace(const char* str, int length) {
  ASSERT(data_ && !OwnsData());
  size_ += length;
}

void JsonStringBuilder::Convert() {
  if (OwnsData())
    return;

  capacity_ = size_;
  if (size_ > 0) {
    const char* old_ptr = data_;
    data_ = Allocate<char>(capacity_);
    uninitializedCopy(data_, old_ptr, size_);
  } else {
    data_ = nullptr;
  }
}

char* JsonStringBuilder::AppendUninitialized(int n) {
  int new_length = size_ + n;
  if (capacity_ < new_length) {
    capacity_ = RecommendCapacity(new_length);
    data_ = Reallocate(data_, capacity_);
  }
  return data_ + exchange(size_, new_length);
}

int JsonStringBuilder::RecommendCapacity(int new_length) {
  constexpr int MinCapacity = 8;

  int rv = (new_length > capacity_ + 4) ? new_length : (new_length * 2);
  return max(rv, MinCapacity);
}

} // namespace stp
