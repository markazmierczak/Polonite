// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/String/StringBuilder.h"

namespace stp {

void StringBuilder::clear() noexcept {
  destroy(exchange(data_, nullptr));
  length_ = 0;
  capacity_ = 0;
}

} // namespace stp
