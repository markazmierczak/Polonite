// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Error/SystemException.h"

#include "Base/Containers/Span.h"

namespace stp {

SystemException::~SystemException() {
}

StringSpan SystemException::getName() const noexcept {
  return "SystemException";
}

void SystemException::onFormat(TextWriter& out) const {
  out << error_code_;
}

} // namespace stp
