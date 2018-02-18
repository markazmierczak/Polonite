// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/Version.h"

#include "Base/Containers/ContiguousAlgo.h"
#include "Base/Type/Comparable.h"
#include "Base/Type/Formattable.h"
#include "Base/Type/Hashable.h"
#include "Base/Type/ParseInteger.h"

namespace stp {

void Version::SetComponentAt(int at, int value) {
  ASSERT(value >= 0);
  if (at >= components_.size())
    components_.AddRepeat(0, at - components_.size() + 1);
  components_[at] = value;
}

int Version::CompareTo(const Version& other) const {
  int min_count = Min(components_.size(), other.components_.size());

  int rv = Compare(
      components_.GetSlice(0, min_count),
      other.components_.GetSlice(0, min_count));

  if (rv)
    return rv;

  if (components_.size() != other.components_.size()) {
    int max_count = Max(components_.size(), other.components_.size());
    for (int i = min_count; i < max_count; ++i) {
      rv = Compare(GetComponentAt(i), other.GetComponentAt(i));
      if (rv)
        return rv;
    }
  }
  return 0;
}

HashCode Version::HashImpl() const {
  return Hash(components_);
}

void Version::FormatImpl(TextWriter& out) const {
  for (int i = 0, e = components_.size(); i < e; ++i) {
    if (i != 0)
      out << '.';
    out << components_[i];
  }
}

bool TryParse(StringSpan str, Version& out) {
  out.components_.Clear();

  int index = 0;
  for (; !str.IsEmpty(); ++index) {
    int dot_pos = str.IndexOf('.');

    StringSpan part;
    if (dot_pos < 0) {
      swap(part, str);
    } else {
      part = str.GetSlice(0, dot_pos);
      str.RemovePrefix(dot_pos + 1);
      // Dot at end ?
      if (str.IsEmpty())
        return false;
    }

    if (StartsWith(part, "+"))
      return false;

    int component;
    if (TryParse(part, component) != ParseIntegerErrorCode::Ok)
      return false;
    if (component < 0)
      return false;
    out.components_.Add(component);
  }

  if (index == 0) {
    // At least one number must be parsed.
    return false;
  }
  return true;
}

} // namespace stp
