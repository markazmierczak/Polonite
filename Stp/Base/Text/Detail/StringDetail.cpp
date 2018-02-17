// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/Detail/StringDetail.h"

#include "Base/Containers/ArrayOps.h"
#include "Base/Containers/BitArray.h"
#include "Base/Io/TextWriter.h"
#include "Base/Memory/Allocate.h"

namespace stp {
namespace detail {

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

} // namespace detail
} // namespace stp
