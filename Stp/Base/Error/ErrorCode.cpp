// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Error/ErrorCode.h"

#include "Base/Compiler/Os.h"
#include "Base/Type/Comparable.h"
#include "Base/Type/Formattable.h"
#include "Base/Type/Hashable.h"

namespace stp {

int compare(const ErrorCode& l, const ErrorCode& r) {
  int rv = compare(&l.getCategory(), &r.getCategory());
  if (!rv)
    rv = compare(l.getCode(), r.getCode());
  return rv;
}

HashCode partialHash(const ErrorCode& x) {
  return partialHashMany(&x.getCategory(), x.getCode());
}

namespace detail {

void format(TextWriter& out, const ErrorCode& x) {
  if (isOk(x)) {
    out << "no error";
  } else {
    x.getCategory().formatMessage(out, x.getCode());
  }
}

StringSpan SuccessErrorCategory::getName() const {
  return "success";
}

void SuccessErrorCategory::formatMessage(TextWriter& out, int code) const {
  out << "success";
}

const SuccessErrorCategory SuccessErrorCategoryInstance;

} // namespace detail

} // namespace stp
