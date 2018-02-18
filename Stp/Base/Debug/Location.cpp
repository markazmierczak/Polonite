// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/Location.h"

#include "Base/Type/Formattable.h"

namespace stp {

/**
 * @fn Location::Location()
 * Construct an unknown location.
 */

/**
 * @fn Location::Location(const char* function_name, const char* file_name, int line_number)
 * Construct a specified location.
 * @a function_name and @a file_name must outlive the constructed object.
 */

TextWriter& operator<<(TextWriter& out, const Location& x) {
  return out << x.getFunctionName() << '@' << x.getFileName() << ':' << x.getLineNumber();
}

} // namespace stp
