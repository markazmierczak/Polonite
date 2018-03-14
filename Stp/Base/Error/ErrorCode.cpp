// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Error/ErrorCode.h"

#include "Base/Compiler/Os.h"
#include "Base/Type/Comparable.h"
#include "Base/Type/Formattable.h"
#include "Base/Type/Hashable.h"

namespace stp {

int compare(const ErrorCode& l, const ErrorCode& r) noexcept {
  int rv = compare(&l.category(), &r.category());
  if (!rv)
    rv = compare(l.code(), r.code());
  return rv;
}

HashCode partialHash(const ErrorCode& x) noexcept {
  return partialHashMany(&x.category(), x.code());
}

namespace detail {

void format(TextWriter& out, const ErrorCode& x) {
  if (isOk(x)) {
    out << "no error";
  } else {
    x.category().formatMessage(out, x.code());
  }
}

StringSpan SuccessErrorCategory::getName() const noexcept {
  return "success";
}

void SuccessErrorCategory::formatMessage(TextWriter& out, int code) const {
  out << "success";
}

const SuccessErrorCategory SuccessErrorCategoryInstance;

} // namespace detail

} // namespace stp
