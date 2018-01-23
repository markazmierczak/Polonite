// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Random/CryptoRandom.h"

#include "Base/Debug/Assert.h"

#include <windows.h>

// #define needed to link in RtlGenRandom(), a.k.a. SystemFunction036.  See the
// "Community Additions" comment on MSDN here:
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa387694.aspx
#define SystemFunction036 NTAPI SystemFunction036
#include <NTSecAPI.h>
#undef SystemFunction036

namespace stp {

void CryptoRandom::NextBytes(byte_t* output, int size) {
  bool success = RtlGenRandom(output, size) != FALSE;
  ASSERT_UNUSED(success, success);
}

} // namespace stp
