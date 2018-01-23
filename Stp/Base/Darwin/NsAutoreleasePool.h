// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_DARWIN_NSAUTORELEASEPOOL_H_
#define STP_BASE_DARWIN_NSAUTORELEASEPOOL_H_

#include "Base/Export.h"
#include "Base/Type/Attributes.h"

#if defined(__OBJC__)
@class NSAutoreleasePool;
#else // __OBJC__
class NSAutoreleasePool;
#endif // __OBJC__

namespace stp {
namespace cocoa {

BASE_EXPORT NSAutoreleasePool* NSAutoreleasePoolAlloc() WARN_UNUSED_RESULT;
BASE_EXPORT void NSAutoreleasePoolDrain(NSAutoreleasePool* pool);

// ScopedNSAutoreleasePool allocates an NSAutoreleasePool when instantiated and
// sends it a -drain message when destroyed.  This allows an autorelease pool to
// be maintained in ordinary C++ code without bringing in any direct Objective-C
// dependency.

class BASE_EXPORT ScopedNSAutoreleasePool {
 public:
  ScopedNSAutoreleasePool() {
    autorelease_pool_ = NSAutoreleasePoolAlloc();
  }

  ~ScopedNSAutoreleasePool() {
    NSAutoreleasePoolDrain(autorelease_pool_);
  }

  // Clear out the pool in case its position on the stack causes it to be
  // alive for long periods of time (such as the entire length of the app).
  // Only use then when you're certain the items currently in the pool are
  // no longer needed.
  void Recycle() {
    NSAutoreleasePoolDrain(autorelease_pool_);
    autorelease_pool_ = NSAutoreleasePoolAlloc();
  }

 private:
  NSAutoreleasePool* autorelease_pool_;

 private:
  DISALLOW_COPY_AND_ASSIGN(ScopedNSAutoreleasePool);
};

} // namespace cocoa
} // namespace stp

#endif // STP_BASE_DARWIN_NSAUTORELEASEPOOL_H_
