// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_DEBUG_LOCATION_H_
#define STP_BASE_DEBUG_LOCATION_H_

#include "Base/Containers/Span.h"

/** @file */

namespace stp {

class BASE_EXPORT Location {
 public:
  constexpr Location() noexcept;
  constexpr Location(const char* function_name, const char* file_name, int line_number) noexcept;
  constexpr Location(const Location& other) = default;

  StringSpan getFunctionName() const;
  StringSpan getFileName() const;
  int getLineNumber() const { return line_number_; }

 private:
  const char* function_name_;
  const char* file_name_;
  int line_number_;
};

BASE_EXPORT TextWriter& operator<<(TextWriter& out, const Location& x);
inline void Format(TextWriter& out, const Location& x, const StringSpan& opts) { out << x; }

/** A macro to record the current source location. */
#define CURRENT_SOURCE_LOCATION() \
    stp::Location(__func__, __FILE__, __LINE__)

constexpr Location::Location(
    const char* function_name, const char* file_name, int line_number) noexcept
    : function_name_(function_name), file_name_(file_name), line_number_(line_number) {}

constexpr Location::Location() noexcept
    : function_name_("unknown"), file_name_("unknown"), line_number_(-1) {}

inline StringSpan Location::getFunctionName() const {
  return MakeSpanFromNullTerminated(function_name_);
}
inline StringSpan Location::getFileName() const {
  return MakeSpanFromNullTerminated(file_name_);
}

} // namespace stp

#endif // STP_BASE_DEBUG_LOCATION_H_
