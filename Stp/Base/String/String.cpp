// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/String/String.h"

#include "Base/Type/Comparable.h"

namespace stp {

/**
 * @fn String::String()
 * Construct a null string (not an empty string).
 */

/**
 * @fn String::String(StringSpan text)
 * Construct a UTF-8 string.
 * A copy of given |text| is made for new string.
 */

bool operator==(const String& lhs, const String& rhs) noexcept {
  if (lhs.isNull() || rhs.isNull())
    return lhs.isNull() == rhs.isNull();
  if (lhs.length() != rhs.length())
    return false;
  return ::memcmp(lhs.data(), rhs.data(), lhs.length()) == 0;
}

int compare(const String& lhs, String& rhs) noexcept {
  if (lhs.isNull() || rhs.isNull())
    return compare(lhs.isNull(), rhs.isNull());
  int common_length = min(lhs.length(), rhs.length());
  int rv = ::memcmp(lhs.data(), rhs.data(), toUnsigned(common_length));
  if (rv)
    return rv;
  return compare(lhs.length(), rhs.length());

}

} // namespace stp
