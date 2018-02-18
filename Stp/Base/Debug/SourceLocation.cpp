// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/SourceLocation.h"

#include "Base/Type/Comparable.h"
#include "Base/Type/Formattable.h"
#include "Base/Type/Hashable.h"

namespace stp {

StringSpan SourceLocation::GetFunctionName() const {
  return MakeSpanFromNullTerminated(function_name_);
}
StringSpan SourceLocation::GetFileName() const {
  return MakeSpanFromNullTerminated(file_name_);
}

int SourceLocation::CompareTo(const SourceLocation& other) const {
  int diff = compare(file_name_, other.file_name_);
  if (!diff)
    diff = compare(line_number_, other.line_number_);
  return diff;
}

HashCode SourceLocation::HashImpl() const {
  return Hash(function_name_);
}

TextWriter& operator<<(TextWriter& out, const SourceLocation& x) {
  return out << x.GetFunctionName() << '@' << x.GetFileName() << ':' << x.GetLineNumber();
}

} // namespace stp
