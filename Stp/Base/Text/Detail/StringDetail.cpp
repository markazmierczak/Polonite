// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/Detail/StringDetail.h"

#include "Base/Containers/ArrayOps.h"
#include "Base/Containers/BitArray.h"
#include "Base/Io/TextWriter.h"
#include "Base/Math/Alignment.h"
#include "Base/Memory/Allocate.h"

namespace stp {
namespace detail {

int GetLengthOfCString(const char16_t* str) {
  ASSERT(str);
  int length = 0;
  for (; *str; ++str)
    ++length;
  return length;
}

namespace {

typedef BitArray<256> CharLookupTable;

// The result table has one on `i`-th position set if |str| contains i-th character.
inline CharLookupTable BuildLookupTable(const char* str, int size) {
  CharLookupTable table;
  for (int i = 0; i < size; ++i)
    table.Set(static_cast<unsigned char>(str[i]));
  return table;
}

} // namespace

int IndexOfAnyCharacter(const char* s, int slength, const char* a, int alength) {
  ASSERT(slength >= 0 && alength >= 0);
  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (alength <= 1)
    return alength != 0 ? MakeSpan(s, slength).IndexOf(*a) : -1;

  CharLookupTable lookup = BuildLookupTable(a, alength);
  for (int i = 0; i < slength; ++i) {
    if (lookup[static_cast<unsigned char>(s[i])])
      return i;
  }
  return -1;
}

int LastIndexOfAnyCharacter(const char* s, int slength, const char* a, int alength) {
  ASSERT(slength >= 0 && alength >= 0);
  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (alength <= 1)
    return alength != 0 ? MakeSpan(s, slength).LastIndexOf(*a) : -1;

  CharLookupTable lookup = BuildLookupTable(a, alength);
  for (int i = slength - 1; i >= 0; --i) {
    if (lookup[static_cast<unsigned char>(s[i])])
      return i;
  }
  return -1;
}

int IndexOfAnyCharacter(const char16_t* s, int slength, const char16_t* a, int alength) {
  ASSERT(slength >= 0 && alength >= 0);
  for (int i = 0; i < slength; ++i) {
    for (int j = 0; j < alength; ++j) {
      if (s[i] == a[j])
        return i;
    }
  }
  return -1;
}

int LastIndexOfAnyCharacter(const char16_t* s, int slength, const char16_t* a, int alength) {
  ASSERT(slength >= 0 && alength >= 0);
  for (int i = slength - 1; i >= 0; --i) {
    for (int j = 0; j < alength; ++j) {
      if (s[i] == a[j])
        return i;
    }
  }
  return -1;
}

int IndexOfAnyCharacterBut(const char* s, int slength, const char* a, int alength) {
  ASSERT(slength >= 0 && alength >= 0);
  for (int i = 0; i < slength; ++i) {
    bool found = false;
    for (int j = 0; j < alength; ++j) {
      if (s[i] == a[j]) {
        found = true;
        break;
      }
    }
    if (!found)
      return i;
  }
  return -1;
}

int LastIndexOfAnyCharacterBut(const char* s, int slength, const char* a, int alength) {
  ASSERT(slength >= 0 && alength >= 0);
  for (int i = slength - 1; i >= 0; --i) {
    bool found = false;
    for (int j = 0; j < alength; ++j) {
      if (s[i] != a[j]) {
        found = true;
        break;
      }
    }
    if (!found)
      return i;
  }
  return -1;
}

template<class T>
static constexpr uint64_t GetNonAsciiMask() {
  return sizeof(T) == 2
      ? UINT64_C(0xFF80FF80FF80FF80)
      : UINT64_C(0x8080808080808080);
}

template<typename T>
static inline bool IsAsciiTmpl(const T* str, int size) {
  ASSERT(size >= 0);
  using MachineWord = uintptr_t;

  MachineWord all_char_bits = 0;
  const T* end = str + size;

  // Prologue: align the input.
  while (!IsAlignedTo(str, sizeof(MachineWord)) && str != end) {
    all_char_bits |= *str;
    ++str;
  }

  // Compare the values of CPU word size.
  constexpr int LoopIncrement = isizeof(MachineWord) / isizeof(T);
  const T* word_end = AlignBackward(end, sizeof(MachineWord));
  for (; str < word_end; str += LoopIncrement)
    all_char_bits |= *(reinterpret_cast<const MachineWord*>(str));

  // Process the remaining bytes.
  while (str != end) {
    all_char_bits |= *str;
    ++str;
  }

  constexpr MachineWord non_ascii_bit_mask = GetNonAsciiMask<T>();
  return !(all_char_bits & non_ascii_bit_mask);
}

bool IsAscii(const char* str, int size) { return IsAsciiTmpl(str, size); }
bool IsAscii(const char16_t* str, int size)  { return IsAsciiTmpl(str, size); }

} // namespace detail
} // namespace stp
