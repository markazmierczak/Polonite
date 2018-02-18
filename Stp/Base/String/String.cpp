// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/String/String.h"

namespace stp {

// Construct a UTF-8 string.
// A copy of given |text| is made for new string.
String::String(StringSpan text)
    : impl_(StringImpl::create(text)) {
}

} // namespace stp
