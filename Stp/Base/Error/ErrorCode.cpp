// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Error/ErrorCode.h"

#include "Base/Compiler/Os.h"
#include "Base/Type/Comparable.h"
#include "Base/Type/Formattable.h"
#include "Base/Type/Hashable.h"

namespace stp {

int compare(const ErrorCode& l, const ErrorCode& r) noexcept {
  int rv = compare(&l.GetCategory(), &r.GetCategory());
  if (!rv)
    rv = compare(l.GetCode(), r.GetCode());
  return rv;
}

HashCode partialHash(const ErrorCode& x) noexcept {
  return partialHashMany(&x.GetCategory(), x.GetCode());
}

namespace detail {

void Format(TextWriter& out, const ErrorCode& x) {
  if (IsOk(x)) {
    out << "no error";
  } else {
    x.GetCategory().FormatMessage(out, x.GetCode());
  }
}

StringSpan SuccessErrorCategory::GetName() const noexcept {
  return "success";
}

void SuccessErrorCategory::FormatMessage(TextWriter& out, int code) const {
  out << "success";
}

const SuccessErrorCategory SuccessErrorCategoryInstance;

} // namespace detail

} // namespace stp
