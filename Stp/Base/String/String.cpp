// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/String/String.h"

#include "Base/Type/Comparable.h"

namespace stp {

/**
 * @fn String::String(StringSpan text)
 * Construct a UTF-8 string.
 * A copy of given |text| is made for new string.
 */

String String::isolate(String s) {
  // Whether impl is safe to be moved to another thread ?
  if (s.impl_->isStatic() ||
      (s.impl_->hasOneRef() && !s.impl_->isInterned())) {
    return move(s);
  }
  return String(StringSpan(s));
}

/**
 * Do not use this function for string literals.
 * @param cstr Null-terminated string UTF-8 encoded.
 * @return A new string value with copy of
 */
String String::fromCString(const char* cstr) {
  int length = static_cast<int>(::strlen(cstr));
  return StringImpl::createFromCString(cstr, length);
}

bool operator==(const String& lhs, const String& rhs) noexcept {
  if (lhs.length() != rhs.length())
    return false;
  return ::memcmp(lhs.data(), rhs.data(), lhs.length()) == 0;
}

int compare(const String& lhs, String& rhs) noexcept {
  int common_length = min(lhs.length(), rhs.length());
  int rv = ::memcmp(lhs.data(), rhs.data(), toUnsigned(common_length));
  if (rv)
    return rv;
  return compare(lhs.length(), rhs.length());

}

} // namespace stp
