// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2010 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef DOUBLE_CONVERSION_UTILS_H_
#define DOUBLE_CONVERSION_UTILS_H_

#include "Base/Compiler/Cpu.h"
#include "Base/Compiler/Os.h"
#include "Base/Containers/Span.h"

#include <string.h>

// Double operations detection based on target architecture.
// Linux uses a 80bit wide floating point stack on x86. This induces double
// rounding, which in turn leads to wrong results.
// An easy way to test if the floating-point operations are correct is to
// evaluate: 89255.0/1e22. If the floating-point stack is 64 bits wide then
// the result is equal to 89255e-22.
// The best way to test this, is to create a division-function and to compare
// the output of the division with the expected result. (Inlining must be disabled.)
// On Linux,x86 89255e-22 != Div_double(89255.0/1e22)
#if CPU(X86_64) || CPU(ARM_FAMILY)
#define DOUBLE_CONVERSION_CORRECT_DOUBLE_OPERATIONS 1
#elif CPU(X86_32)
#if OS(WIN)
// Windows uses a 64bit wide floating point stack.
#define DOUBLE_CONVERSION_CORRECT_DOUBLE_OPERATIONS 1
#else
#undef DOUBLE_CONVERSION_CORRECT_DOUBLE_OPERATIONS
#endif // _WIN32
#else
#error "target architecture was not detected as supported by dtoa"
#endif

// The following macro works on both 32 and 64-bit platforms.
// Usage: instead of writing 0x1234567890123456
//      write UINT64_2PART_C(0x12345678,90123456);
#define UINT64_2PART_C(a, b) (((static_cast<uint64_t>(a) << 32) + 0x##b##u))

namespace stp {

namespace dtoa {

// Returns the maximum of the two parameters.
template<typename T>
static T max(T a, T b) {
  return a < b ? b : a;
}

// Returns the minimum of the two parameters.
template<typename T>
static T min(T a, T b) {
  return a < b ? a : b;
}

inline int StrLength(const char* string) {
  return static_cast<int>(strlen(string));
}

// This is a simplified version of V8's Vector class.
template<typename T>
class Vector {
 public:
  Vector() : start_(NULL), length_(0) {}
  Vector(T* data, int length) : start_(data), length_(length) {
    ASSERT(length == 0 || (length > 0 && data != NULL));
  }

  // Returns a vector using the same backing storage as this one,
  // spanning from and including 'from', to but not including 'to'.
  Vector<T> SubVector(int from, int to) {
    ASSERT(to <= length_);
    ASSERT(from < to);
    ASSERT(0 <= from);
    return Vector<T>(start() + from, to - from);
  }

  // Returns the length of the vector.
  int length() const { return length_; }

  // Returns whether or not the vector is empty.
  bool is_empty() const { return length_ == 0; }

  // Returns the pointer to the start of the data in the vector.
  T* start() const { return start_; }

  // Access individual vector elements.
  T& operator[](int index) const {
    ASSERT(0 <= index && index < length_);
    return start_[index];
  }

  T& first() { return start_[0]; }

  T& last() { return start_[length_ - 1]; }

 private:
  T* start_;
  int length_;
};

// Helper class for building result strings in a character buffer. The
// purpose of the class is to use safe operations that checks the
// buffer bounds on all operations in debug mode.
class StringBuilder {
 public:
  StringBuilder(char* buffer, int size) : buffer_(buffer, size), position_(0) {}

  ~StringBuilder() {
    if (!is_finalized())
      finalizeHash();
  }

  int size() const { return buffer_.length(); }

  // Get the current position in the builder.
  int position() const {
    ASSERT(!is_finalized());
    return position_;
  }

  // Set the current position in the builder.
  void SetPosition(int position) {
    ASSERT(!is_finalized());
    ASSERT(position < size());
    position_ = position;
  }

  // Reset the position.
  void Reset() { position_ = 0; }

  // Add a single character to the builder. It is not allowed to add
  // 0-characters; use the finalizeHash() method to terminate the string
  // instead.
  void AddCharacter(char c) {
    ASSERT(c != '\0');
    ASSERT(!is_finalized());
    ASSERT(position_ < buffer_.length());
    buffer_[position_++] = c;
  }

  // Add an entire string to the builder. Uses strlen() internally to
  // compute the length of the input string.
  void AddString(const char* s) { AddSubstring(s, StrLength(s)); }

  // Add the first 'n' characters of the given string 's' to the
  // builder. The input string must have enough characters.
  void AddSubstring(const char* s, int n) {
    ASSERT(!is_finalized());
    ASSERT(position_ + n < buffer_.length());
    ASSERT(n <= StrLength(s));
    memcpy(&buffer_[position_], s, n);
    position_ += n;
  }

  // Add character padding to the builder. If count is non-positive,
  // nothing is added to the builder.
  void AddPadding(char c, int count) {
    for (int i = 0; i < count; i++) {
      AddCharacter(c);
    }
  }

  // Finalize the string by 0-terminating it and returning the buffer.
  StringSpan finalizeHash() {
    ASSERT(!is_finalized());
    ASSERT(position_ < buffer_.length());
    buffer_[position_] = '\0';
    // Make sure nobody managed to add a 0-character to the
    // buffer while building the string.
    ASSERT(StrLength(buffer_.start()) == position_);
    StringSpan result(buffer_.start(), position_);
    position_ = -1;
    return result;
  }

 private:
  Vector<char> buffer_;
  int position_;

  bool is_finalized() const { return position_ < 0; }

  DISALLOW_COPY_AND_ASSIGN(StringBuilder);
};

} // namespace dtoa

} // namespace stp

#endif // DOUBLE_CONVERSION_UTILS_H_
