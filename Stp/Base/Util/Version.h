// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_VERSION_H_
#define STP_BASE_UTIL_VERSION_H_

#include "Base/Containers/InlineList.h"
#include "Base/Type/Parsable.h"

namespace stp {

class Version;

BASE_EXPORT bool TryParse(StringSpan text, Version& version);

// Version represents a dotted version number, like "1.2.3.4", supporting
// parsing and comparison.
class BASE_EXPORT Version {
 public:
  Version() = default;
  Version(int major_part, int minor_part, int micro_part)
      : components_({major_part, minor_part, micro_part}) {}

  int GetMajor() const { return GetComponentAt(0); }
  int GetMinor() const { return GetComponentAt(1); }
  int GetMicro() const { return GetComponentAt(2); }

  void SetComponentAt(int at, int value);
  int GetComponentAt(int at) const { return at < components_.size() ? components_[at] : 0; }

  bool operator==(const Version& other) const { return CompareTo(other) == 0; }
  bool operator!=(const Version& other) const { return CompareTo(other) != 0; }
  bool operator<=(const Version& other) const { return CompareTo(other) <= 0; }
  bool operator>=(const Version& other) const { return CompareTo(other) >= 0; }
  bool operator< (const Version& other) const { return CompareTo(other) <  0; }
  bool operator> (const Version& other) const { return CompareTo(other) >  0; }
  friend int Compare(const Version& l, const Version& r) { return l.CompareTo(r); }
  friend HashCode Hash(const Version& x) { return x.HashImpl(); }

  friend TextWriter& operator<<(TextWriter& out, const Version& x) {
    x.FormatImpl(out); return out;
  }
  friend void Format(TextWriter& out, const Version& x, const StringSpan& opts) {
    x.FormatImpl(out);
  }

  friend bool TryParse(StringSpan text, Version& out);

 private:
  int CompareTo(const Version& other) const;
  void FormatImpl(TextWriter& out) const;
  HashCode HashImpl() const;

  InlineList<int, 4> components_;
};

} // namespace stp

#endif // STP_BASE_UTIL_VERSION_H_
