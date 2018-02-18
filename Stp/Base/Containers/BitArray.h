// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_BITARRAY_H_
#define STP_BASE_CONTAINERS_BITARRAY_H_

#include "Base/Debug/Assert.h"
#include "Base/Math/Bits.h"
#include "Base/Type/FormattableFwd.h"
#include "Base/Type/Variable.h"

#include <string.h>

namespace stp {

namespace detail {

BASE_EXPORT void FormatBitArray(
    TextWriter& out, const StringSpan& opts, const uintptr_t* words, int size);

} // namespace detail

class BitReference {
 public:
  // uintptr_t is selected as Word type for maximum performance.
  typedef uintptr_t WordType;

  BitReference(WordType* word, WordType bit) noexcept
      : word_(word), bit_(bit) {}

  BitReference(const BitReference&) = default;

  BitReference& operator=(const BitReference& x) {
    return operator=(static_cast<bool>(x));
  }

  BitReference& operator=(bool v) {
    if (v) {
      *word_ |= bit_;
    } else {
      *word_ &= ~bit_;
    }
    return *this;
  }

  operator bool() const { return (*word_ & bit_) != 0; }
  bool operator~() const { return (*word_ & bit_) == 0; }

 private:
  WordType* word_;
  WordType bit_;
};

class ConstBitReference {
 public:
  typedef BitReference::WordType Word;

  ConstBitReference(const Word* word, Word bit)
      : word_(word), bit_(bit) {}

  operator bool() const { return (*word_ & bit_) != 0; }
  bool operator~() const { return (*word_ & bit_) == 0; }

 private:
  const Word* word_;
  Word bit_;
};

// NOTE: memset and memcpy is not used in this class since BitArray is frequently
// used for small numbers of N.
template<int N>
class BitArray {
 public:
  static_assert(N > 0, "!");

  constexpr BitArray() noexcept : words_() {}

  explicit constexpr BitArray(uint64_t x) noexcept;

  bool operator[](int index) const { return MakeRef(index); }
  BitReference operator[](int index) { return MakeRef(index); }

  ALWAYS_INLINE constexpr int size() const { return N; }

  bool Test(int index) const { return MakeRef(index); }
  bool Test(const BitArray& other) const;

  void Set(int index) { MakeRef(index) = true; }
  void Unset(int index) { MakeRef(index) = false; }
  void Flip(int index);

  void SetAll();
  void UnsetAll();
  void FlipAll();

  int Count() const;

  int FindFirstSet();
  int FindNextSet(int prev);

  int FindLastSet();
  int FindPrevSet(int next);

  // |amount| must be lower or equal to |Size|.
  BitArray& operator>>=(int amount);
  BitArray& operator<<=(int amount);

  BitArray& operator&=(const BitArray& other);
  BitArray& operator|=(const BitArray& other);
  BitArray& operator^=(const BitArray& other);
  BitArray operator~() const;

  bool AllTrue() const;
  bool AnyTrue() const;

  friend BitArray operator>>(const BitArray& x, int amount) {
    BitArray rv = x; rv >>= amount; return rv;
  }
  friend BitArray operator<<(const BitArray& x, int amount) {
    BitArray rv = x; rv <<= amount; return rv;
  }
  friend BitArray operator&(const BitArray& lhs, const BitArray& rhs) {
    BitArray rv = lhs; rv &= rhs; return rv;
  }
  friend BitArray operator|(const BitArray& lhs, const BitArray& rhs) {
    BitArray rv = lhs; rv |= rhs; return rv;
  }
  friend BitArray operator^(const BitArray& lhs, const BitArray& rhs) {
    BitArray rv = lhs; rv ^= rhs; return rv;
  }

  friend void swap(BitArray& l, BitArray& r) noexcept { swap(l.words_, r.words_); }
  friend bool operator==(const BitArray& l, const BitArray& r) {
    return ::memcmp(l.words_, r.words_, sizeof(words_)) == 0;
  }
  friend bool operator!=(const BitArray& l, const BitArray& r) { return !operator==(l, r); }
  friend int compare(const BitArray& l, const BitArray& r) {
    return ::memcmp(l.words_, r.words_, sizeof(words_));
  }
  friend HashCode Hash(const BitArray& x) { return HashBuffer(x.words_, N * isizeof(WordType)); }
  friend void Format(TextWriter& out, const BitArray& x, const StringSpan& opts) {
    detail::FormatBitArray(out, opts, x.words_, N);
  }

 private:
  typedef BitReference::WordType WordType;

  // Make it unsigned to force fast modulo for power-of-2 divisor.
  static constexpr unsigned BitsPerWord = sizeof(WordType) * 8;

  static constexpr int WordCount = (N + BitsPerWord - 1) / BitsPerWord;

  WordType words_[WordCount];

  static constexpr WordType ComputeUnusedBits() {
    unsigned used_bits = N % BitsPerWord;
    if (used_bits == 0)
      return 0; // All bits used.
    return ~((WordType(1) << used_bits) - 1);
  }

  static constexpr WordType UnusedBitsMask = ComputeUnusedBits();

  ConstBitReference MakeRef(unsigned index) const {
    ASSERT(index < static_cast<unsigned>(N));
    return ConstBitReference(words_ + index / BitsPerWord, WordType(1) << (index % BitsPerWord));
  }

  BitReference MakeRef(unsigned index) {
    ASSERT(index < static_cast<unsigned>(N));
    return BitReference(words_ + index / BitsPerWord, WordType(1) << (index % BitsPerWord));
  }

  constexpr void ClearUnusedBits() {
    if (UnusedBitsMask != 0)
      words_[WordCount - 1] &= ~UnusedBitsMask;
  }
};

template<int N>
struct TIsZeroConstructibleTmpl<BitArray<N>> : TTrue {};
template<int N>
struct TIsTriviallyRelocatableTmpl<BitArray<N>> : TTrue {};
template<int N>
struct TIsTriviallyEqualityComparableTmpl<BitArray<N>> : TTrue {};

template<int N>
constexpr BitArray<N>::BitArray(uint64_t x) noexcept : words_() {
  for (int i = 0; i < WordCount; ++i) {
    if (sizeof(WordType) == sizeof(uint64_t)) {
      words_[i] = x;
    } else if (sizeof(WordType) == sizeof(uint32_t)) {
      if (i & 1)
        words_[i] = static_cast<uint32_t>(x >> 32);
      else
        words_[i] = static_cast<uint32_t>(x >> 0);
    } else {
      ASSERT(false);
    }
  }
  ClearUnusedBits();
}

template<int N>
inline bool BitArray<N>::Test(const BitArray& other) const {
  for (int i = 0; i < WordCount; ++i) {
    if (words_[i] & other.words_[i])
      return true;
  }
  return false;
}

template<int N>
inline void BitArray<N>::Flip(int index) {
  BitReference ref = MakeRef(index);
  ref = !static_cast<bool>(ref);
}

template<int N>
inline void BitArray<N>::SetAll() {
  for (int i = 0; i < WordCount; ++i)
    words_[i] = static_cast<WordType>(-1);
  ClearUnusedBits();
}

template<int N>
inline void BitArray<N>::UnsetAll() {
  for (int i = 0; i < WordCount; ++i)
    words_[i] = 0;
}

template<int N>
inline void BitArray<N>::FlipAll() {
  for (int i = 0; i < WordCount; ++i)
    words_[i] = ~words_[i];
  ClearUnusedBits();
}

template<int N>
inline int BitArray<N>::Count() const {
  int count = 0;
  for (int i = 0; i < WordCount; ++i)
    count += CountBitsPopulation(words_[i]);
  return count;
}

template<int N>
inline BitArray<N>& BitArray<N>::operator&=(const BitArray& other) {
  for (int i = 0; i < WordCount; ++i)
    words_[i] &= other.words_[i];
  return *this;
}

template<int N>
inline BitArray<N>& BitArray<N>::operator|=(const BitArray& other) {
  for (int i = 0; i < WordCount; ++i)
    words_[i] |= other.words_[i];
  return *this;
}

template<int N>
inline BitArray<N>& BitArray<N>::operator^=(const BitArray& other) {
  for (int i = 0; i < WordCount; ++i)
    words_[i] ^= other.words_[i];
  return *this;
}

template<int N>
inline BitArray<N> BitArray<N>::operator~() const {
  BitArray result = *this;
  result.FlipAll();
  return result;
}

template<int N>
inline int BitArray<N>::FindFirstSet() {
  for (int i = 0; i < WordCount; ++i) {
    if (words_[i] != 0)
      return i * BitsPerWord + FindFirstOneBit(words_[i]);
  }
  return -1;
}

template<int N>
inline int BitArray<N>::FindNextSet(int prev) {
  ASSERT(0 <= prev && prev < N);

  ++prev;
  if (prev >= N)
    return -1;

  int word_index = prev / BitsPerWord;
  int bit_index = prev % BitsPerWord;
  WordType copy = words_[word_index];

  // Mask off previous bits.
  copy &= ~WordType(0) << bit_index;

  if (copy != 0)
    return word_index * BitsPerWord + FindFirstOneBit(copy);

  // Check subsequent words.
  for (int i = word_index + 1; i < WordCount; ++i) {
    if (words_[i] != 0)
      return i * BitsPerWord + FindFirstOneBit(words_[i]);
  }
  return -1;
}

template<int N>
inline int BitArray<N>::FindLastSet() {
  for (int i = WordCount - 1; i >= 0; --i) {
    if (words_[i] != 0)
      return i * BitsPerWord + FindLastOneBit(words_[i]);
  }
  return -1;
}

template<int N>
inline int BitArray<N>::FindPrevSet(int next) {
  if (next == 0)
    return -1;
  --next;
  ASSERT(0 <= next && next < N);

  int word_index = next / BitsPerWord;
  int bit_index = (next % BitsPerWord) + 1;
  WordType copy = words_[word_index];

  // Mask off previous bits.
  if (bit_index != BitsPerWord)
    copy &= (WordType(1) << bit_index) - 1;

  if (copy != 0)
    return word_index * BitsPerWord + FindLastOneBit(copy);

  // Check subsequent words.
  if (WordCount > 1) {
    for (int i = word_index - 1; i >= 0; --i) {
      if (words_[i] != 0)
        return i * BitsPerWord + FindLastOneBit(words_[i]);
    }
  }
  return -1;
}

template<int N>
inline bool BitArray<N>::AllTrue() const {
  for (int i = 0; i < WordCount - 1; ++i) {
    if (words_[i] != ~static_cast<WordType>(0))
      return false;
  }
  return words_[WordCount - 1] == ~UnusedBitsMask;
}

template<int N>
inline bool BitArray<N>::AnyTrue() const {
  for (int i = 0; i < WordCount; ++i) {
    if (words_[i] != 0)
      return true;
  }
  return false;
}

template<int N>
inline BitArray<N>& BitArray<N>::operator>>=(int amount) {
  ASSERT(amount <= N);

  int word_amount = amount / BitsPerWord;
  int bit_amount = amount % BitsPerWord;
  int limit = WordCount - word_amount - 1;

  if (bit_amount == 0) {
    if (UNLIKELY(word_amount == N)) {
      // Request to fill all bits with zero.
      limit = -1;
    } else {
      for (int i = 0; i <= limit; ++i)
        words_[i] = words_[i + word_amount];
    }
  } else {
    for (int i = 0; i < limit; ++i) {
      words_[i] = ((words_[i + word_amount] >> bit_amount) |
                   (words_[i + word_amount + 1] << (BitsPerWord - bit_amount)));
    }
    words_[limit] = words_[WordCount - 1] >> bit_amount;
  }

  for (int i = limit + 1; i < WordCount; ++i)
    words_[i] = 0;

  return *this;
}

template<int N>
inline BitArray<N>& BitArray<N>::operator<<=(int amount) {
  ASSERT(amount <= N);

  int word_amount = amount / BitsPerWord;
  int bit_amount = amount % BitsPerWord;

  if (bit_amount == 0) {
    for (int i = WordCount - 1; i >= word_amount; --i)
      words_[i] = words_[i - word_amount];
  } else {
    for (int i = WordCount - 1; i > word_amount; --i) {
      words_[i] = ((words_[i - word_amount] << bit_amount) |
                   (words_[i - word_amount - 1] >> (BitsPerWord - bit_amount)));
    }
    words_[word_amount] = words_[0] << bit_amount;
  }

  for (int i = 0; i < word_amount; ++i)
    words_[i] = 0;

  ClearUnusedBits();

  return *this;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_BITARRAY_H_
