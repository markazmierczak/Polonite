// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Error/ErrorCode.h"

#include "Base/Compiler/Os.h"
#include "Base/Type/Comparable.h"
#include "Base/Type/Formattable.h"
#include "Base/Type/Hashable.h"

namespace stp {

int Compare(const ErrorCode& l, const ErrorCode& r) noexcept {
  int rv = Compare(&l.GetCategory(), &r.GetCategory());
  if (!rv)
    rv = Compare(l.GetCode(), r.GetCode());
  return rv;
}

HashCode Hash(const ErrorCode& x) noexcept {
  return HashMany(&x.GetCategory(), x.GetCode());
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
