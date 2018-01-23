// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Darwin/NsAutoreleasePool.h"

#import <Foundation/Foundation.h>

#include "Base/Debug/Assert.h"

namespace stp {
namespace cocoa {

NSAutoreleasePool* NSAutoreleasePoolAlloc() {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  ASSERT(pool);
  return pool;
}

void NSAutoreleasePoolDrain(NSAutoreleasePool* pool) {
  ASSERT(pool);
  [pool drain];
}

} // namespace cocoa
} // namespace stp
