// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/System/CpuInfo.h"

#include "Base/Test/GTest.h"

namespace stp {

TEST(CpuInfoTest, NumProcs) {
  // We aren't actually testing that it's correct, just that it's sane.
  EXPECT_GE(CpuInfo::NumberOfCores(), 1);
}

} // namespace stp
