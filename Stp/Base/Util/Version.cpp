// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/Version.h"

#include "Base/Containers/ContiguousAlgo.h"
#include "Base/Type/Comparable.h"
#include "Base/Type/Formattable.h"
#include "Base/Type/Hashable.h"
#include "Base/Type/ParseInteger.h"

namespace stp {

void Version::setPart(int at, int value) {
  ASSERT(value >= 0);
  if (at >= parts_.size())
    parts_.AddRepeat(0, at - parts_.size() + 1);
  parts_[at] = value;
}

int Version::CompareTo(const Version& other) const {
  int max_count = max(parts_.size(), other.parts_.size());
  for (int i = 0; i < max_count; ++i) {
    int rv = compare(getPartAt(i), other.getPartAt(i));
    if (rv)
      return rv;
  }
  return 0;
}

HashCode Version::HashImpl() const {
  return hashBuffer(parts_.data(), parts_.size() * isizeof(PartType));
}

void Version::FormatImpl(TextWriter& out) const {
  for (int i = 0, e = parts_.size(); i < e; ++i) {
    if (i != 0)
      out << '.';
    out << parts_[i];
  }
}

bool tryParse(StringSpan str, Version& out) {
  out.parts_.Clear();

  int index = 0;
  for (; !str.IsEmpty(); ++index) {
    int dot_pos = str.IndexOf('.');

    StringSpan part_str;
    if (dot_pos < 0) {
      swap(part_str, str);
    } else {
      part_str = str.GetSlice(0, dot_pos);
      str.RemovePrefix(dot_pos + 1);
      // Dot at end ?
      if (str.IsEmpty())
        return false;
    }

    if (StartsWith(part_str, "+"))
      return false;

    int part;
    if (tryParse(part_str, part) != ParseIntegerErrorCode::Ok)
      return false;
    if (part < 0)
      return false;
    out.parts_.Add(part);
  }

  if (index == 0) {
    // At least one number must be parsed.
    return false;
  }
  return true;
}

} // namespace stp
