// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/StringAlgo.h"

#include "Base/Containers/BitArray.h"

namespace stp {

namespace {

typedef BitArray<256> CharLookupTable;

// The result table has one on `i`-th position set if |str| contains i-th character.
inline CharLookupTable BuildLookupTable(StringSpan s) {
  CharLookupTable table;
  for (int i = 0; i < s.size(); ++i)
    table.Set(static_cast<unsigned char>(s[i]));
  return table;
}

} // namespace

int IndexOfAny(StringSpan s, StringSpan a) {
  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (a.size() <= 1)
    return !a.IsEmpty() ? s.IndexOf(a[0]) : -1;

  CharLookupTable lookup = BuildLookupTable(a);
  for (int i = 0; i < s.size(); ++i) {
    if (lookup[static_cast<unsigned char>(s[i])])
      return i;
  }
  return -1;
}

int LastIndexOfAny(StringSpan s, StringSpan a) {
  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (a.size() <= 1)
    return !a.IsEmpty() ? s.LastIndexOf(a[0]) : -1;

  CharLookupTable lookup = BuildLookupTable(a);
  for (int i = s.size() - 1; i >= 0; --i) {
    if (lookup[static_cast<unsigned char>(s[i])])
      return i;
  }
  return -1;
}

int IndexOfAnyBut(StringSpan s, StringSpan a) {
  for (int i = 0; i < s.size(); ++i) {
    bool found = false;
    for (int j = 0; j < a.size(); ++j) {
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

int LastIndexOfAnyBut(StringSpan s, StringSpan a) {
  for (int i = s.size() - 1; i >= 0; --i) {
    bool found = false;
    for (int j = 0; j < a.size(); ++j) {
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

} // namespace stp
