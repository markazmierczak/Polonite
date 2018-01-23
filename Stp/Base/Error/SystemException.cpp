// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Error/SystemException.h"

#include "Base/Text/StringSpan.h"

namespace stp {

StringSpan SystemException::GetName() const noexcept {
  return "SystemException";
}

void SystemException::OnFormat(TextWriter& out) const {
  out << error_code_;
}

} // namespace stp
