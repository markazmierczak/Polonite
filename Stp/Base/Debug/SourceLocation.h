// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_DEBUG_SOURCELOCATION_H_
#define STP_BASE_DEBUG_SOURCELOCATION_H_

#include "Base/Export.h"
#include "Base/Type/Basic.h"
#include "Base/Type/FormattableFwd.h"

namespace stp {

// Location provides basic info where of an object was constructed, or was
// significantly brought to life.
class BASE_EXPORT SourceLocation {
 public:
  // Constructor should be called with a long-lived char*, such as __FILE__.
  // It assumes the provided value will persist as a global constant, and it
  // will not make a copy of it.
  constexpr SourceLocation() noexcept;
  constexpr SourceLocation(const char* function_name, const char* file_name, int line_number) noexcept;

  constexpr SourceLocation(const SourceLocation& other) = default;

  StringSpan GetFunctionName() const;
  StringSpan GetFileName() const;
  int GetLineNumber() const { return line_number_; }

  friend constexpr bool operator==(const SourceLocation& l, const SourceLocation& r) {
    // No need to use |function_name_| since the other two fields uniquely
    // identify this location.
    return l.line_number_ == r.line_number_ && l.file_name_ == r.file_name_;
  }
  friend constexpr bool operator!=(const SourceLocation& l, const SourceLocation& r) {
    return !operator==(l, r);
  }
  friend int compare(const SourceLocation& l, const SourceLocation& r) { return l.CompareTo(r); }
  friend HashCode Hash(const SourceLocation& x) { return x.HashImpl(); }

 private:
  int CompareTo(const SourceLocation& other) const;
  HashCode HashImpl() const;

  const char* function_name_;
  const char* file_name_;
  int line_number_;
};

BASE_EXPORT TextWriter& operator<<(TextWriter& out, const SourceLocation& x);
inline void Format(TextWriter& out, const SourceLocation& x, const StringSpan& opts) { out << x; }

// Define a macro to record the current source location.
#define CURRENT_SOURCE_LOCATION() \
    stp::SourceLocation(__func__, __FILE__, __LINE__)

constexpr SourceLocation::SourceLocation(
    const char* function_name, const char* file_name, int line_number) noexcept
    : function_name_(function_name), file_name_(file_name), line_number_(line_number) {}

constexpr SourceLocation::SourceLocation() noexcept
    : function_name_("unknown"), file_name_("unknown"), line_number_(-1) {}

} // namespace stp

#endif // STP_BASE_DEBUG_SOURCELOCATION_H_
