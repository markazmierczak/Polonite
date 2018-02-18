// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_WIN_SCOPEDCOMINITIALIZER_H_
#define STP_BASE_WIN_SCOPEDCOMINITIALIZER_H_

#include "Base/Debug/Assert.h"

#include <objbase.h>

namespace stp {
namespace win {

// Initializes COM in the constructor (STA or MTA), and uninitializes COM in the
// destructor.
//
// WARNING: This should only be used once per thread, ideally scoped to a
// similar lifetime as the thread itself.  You should not be using this in
// random utility functions that make COM calls -- instead ensure these
// functions are running on a COM-supporting thread!
class ScopedComInitializer {
 public:
  // Enum value provided to initialize the thread as an MTA instead of STA.
  enum SelectMTA { kMTA };

  // Constructor for STA initialization.
  ScopedComInitializer() {
    Initialize(COINIT_APARTMENTTHREADED);
  }

  // Constructor for MTA initialization.
  explicit ScopedComInitializer(SelectMTA mta) {
    Initialize(COINIT_MULTITHREADED);
  }

  ~ScopedComInitializer() {
    // Using the windows API directly to avoid dependency on platform_thread.
    ASSERT(thread_id_ == GetCurrentThreadId());
    if (succeeded())
      CoUninitialize();
  }

  bool succeeded() const { return SUCCEEDED(hr_); }

 private:
  void Initialize(COINIT init) {
    #if ASSERT_IS_ON
    thread_id_ = GetCurrentThreadId();
    #endif
    hr_ = CoInitializeEx(NULL, init);
    #ifndef NDEBUG
    if (hr_ == S_FALSE)
      RELEASE_LOGF("Multiple CoInitialize() calls for thread %ld", thread_id_);
    else
      ASSERTF(hr_  != RPC_E_CHANGED_MODE, "invalid COM thread model change");
    #endif
  }

  HRESULT hr_;
  #ifndef NDEBUG
  // In debug builds we use this variable to catch a potential bug where a
  // ScopedComInitializer instance is deleted on a different thread than it
  // was initially created on.  If that ever happens it can have bad
  // consequences and the cause can be tricky to track down.
  DWORD thread_id_;
  #endif

  DISALLOW_COPY_AND_ASSIGN(ScopedComInitializer);
};

} // namespace win
} // namespace stp

#endif // STP_BASE_WIN_SCOPEDCOMINITIALIZER_H_
