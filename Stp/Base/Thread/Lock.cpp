// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is used for debugging assertion support.
// The BasicLock class is functionally a wrapper around the NativeLock class.

#include "Base/Thread/Lock.h"

#if ASSERT_IS_ON()

namespace stp {

bool BasicLock::TryAcquire() {
  bool rv = NativeLock::TryAcquire(&native_object_);
  if (rv)
    CheckUnheldAndMark();

  return rv;
}

void BasicLock::Acquire() {
  NativeLock::Acquire(&native_object_);
  CheckUnheldAndMark();
}

void BasicLock::Release() {
  CheckHeldAndUnmark();
  NativeLock::Release(&native_object_);
}

void BasicLock::AssertAcquired() const {
  ASSERT(owning_thread_ == NativeThread::CurrentHandle());
}

void BasicLock::CheckHeldAndUnmark() {
  ASSERT(owning_thread_ == NativeThread::CurrentHandle());
  owning_thread_ = InvalidNativeThreadHandle;
}

void BasicLock::CheckUnheldAndMark() {
  ASSERT(owning_thread_ == InvalidNativeThreadHandle);
  owning_thread_ = NativeThread::CurrentHandle();
}

} // namespace stp

#endif // ASSERT_IS_ON()
