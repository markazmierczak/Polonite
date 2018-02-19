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

#ifndef STP_BASE_DTOA_BIGNUM_H_
#define STP_BASE_DTOA_BIGNUM_H_

#include "Base/Dtoa/Utils.h"

namespace stp {

namespace dtoa {

class Bignum {
 public:
  // 3584 = 128 * 28. We can represent 2^3584 > 10^1000 accurately.
  // This bignum can encode much bigger numbers, since it contains an
  // exponent.
  static const int MaxSignificantBits = 3584;

  Bignum();
  void AssignUInt16(uint16_t value);
  void AssignUInt64(uint64_t value);
  void AssignBignum(const Bignum& other);

  void AssignDecimalString(Vector<const char> value);
  void AssignHexString(Vector<const char> value);

  void AssignPowerUInt16(uint16_t base, int exponent);

  void AddUInt16(uint16_t operand);
  void AddUInt64(uint64_t operand);
  void AddBignum(const Bignum& other);
  // Precondition: this >= other.
  void SubtractBignum(const Bignum& other);

  void Square();
  void ShiftLeft(int shift_amount);
  void MultiplyByUInt32(uint32_t factor);
  void MultiplyByUInt64(uint64_t factor);
  void MultiplyByPowerOfTen(int exponent);
  void Times10() { return MultiplyByUInt32(10); }
  // Pseudocode:
  //  int result = this / other;
  //  this = this % other;
  // In the worst case this function is in O(this/other).
  uint16_t DivideModuloIntBignum(const Bignum& other);

  bool ToHexString(char* buffer, int buffer_size) const;

  static int compare(const Bignum& a, const Bignum& b);
  static bool Equal(const Bignum& a, const Bignum& b) {
    return compare(a, b) == 0;
  }
  static bool LessEqual(const Bignum& a, const Bignum& b) {
    return compare(a, b) <= 0;
  }
  static bool Less(const Bignum& a, const Bignum& b) {
    return compare(a, b) < 0;
  }
  // Returns compare(a + b, c);
  static int PlusCompare(const Bignum& a, const Bignum& b, const Bignum& c);
  // Returns a + b == c
  static bool PlusEqual(const Bignum& a, const Bignum& b, const Bignum& c) {
    return PlusCompare(a, b, c) == 0;
  }
  // Returns a + b <= c
  static bool PlusLessEqual(const Bignum& a, const Bignum& b, const Bignum& c) {
    return PlusCompare(a, b, c) <= 0;
  }
  // Returns a + b < c
  static bool PlusLess(const Bignum& a, const Bignum& b, const Bignum& c) {
    return PlusCompare(a, b, c) < 0;
  }

 private:
  typedef uint32_t Chunk;
  typedef uint64_t DoubleChunk;

  static const int ChunkSize = sizeof(Chunk) * 8;
  static const int DoubleChunkSize = sizeof(DoubleChunk) * 8;
  // With bigit size of 28 we loose some bits, but a double still fits easily
  // into two chunks, and more importantly we can use the Comba multiplication.
  static const int BigitSize = 28;
  static const Chunk BigitMask = (1 << BigitSize) - 1;
  // Every instance allocates BigitLength chunks on the stack. Bignums cannot
  // grow. There are no checks if the stack-allocated space is sufficient.
  static const int BigitCapacity = MaxSignificantBits / BigitSize;

  void ensureCapacity(int size) {
    ASSERT(size <= BigitCapacity, "capacity is not enough");
  }
  void Align(const Bignum& other);
  void clamp();
  bool IsClamped() const;
  void Zero();
  // Requires this to have enough capacity (no tests done).
  // Updates used_digits_ if necessary.
  // shift_amount must be < BigitSize.
  void BigitsShiftLeft(int shift_amount);
  // BigitLength includes the "hidden" digits encoded in the exponent.
  int BigitLength() const { return used_digits_ + exponent_; }
  Chunk BigitAt(int index) const;
  void SubtractTimes(const Bignum& other, int factor);

  Chunk bigits_buffer_[BigitCapacity];
  // A vector backed by bigits_buffer_. This way accesses to the array are
  // checked for out-of-bounds errors.
  Vector<Chunk> bigits_;
  int used_digits_;
  // The Bignum's value equals value(bigits_) * 2^(exponent_ * BigitSize).
  int exponent_;

  DISALLOW_COPY_AND_ASSIGN(Bignum);
};

} // namespace dtoa

} // namespace stp

#endif // STP_BASE_DTOA_BIGNUM_H_
