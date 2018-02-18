// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Crypto/CryptoRandom.h"

#include <windows.h>

// #define needed to link in RtlGenRandom(), a.k.a. SystemFunction036.  See the
// "Community Additions" comment on MSDN here:
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa387694.aspx
#define SystemFunction036 NTAPI SystemFunction036
#include <NTSecAPI.h>
#undef SystemFunction036

namespace stp {

void CryptoRandom::generate(MutableBufferSpan buffer) noexcept {
  bool success = RtlGenRandom(buffer.data(), ToUnsigned(buffer.size())) != FALSE;
  ASSERT_UNUSED(success, success);
}

} // namespace stp
