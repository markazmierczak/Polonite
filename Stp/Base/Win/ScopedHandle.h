// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_WIN_SCOPEDHANDLE_H_
#define STP_BASE_WIN_SCOPEDHANDLE_H_

#include "Base/Debug/Assert.h"
#include "Base/Test/GTestProdUtil.h"
#include "Base/Win/WindowsHeader.h"

namespace stp {
namespace win {

class ScopedHandle;

class BASE_EXPORT ScopedHandle {
 public:
  ScopedHandle() noexcept : handle_(INVALID_HANDLE_VALUE) {}

  ~ScopedHandle() {
    if (IsValid())
      CloseHandle(handle_);
  }

  explicit ScopedHandle(HANDLE handle) noexcept
      : handle_(handle) {}

  ScopedHandle(ScopedHandle&& other) noexcept
      : handle_(other.release()) {}

  ScopedHandle& operator=(ScopedHandle&& other) noexcept {
    if (this != &other)
      Reset(other.release());
    return *this;
  }

  void SwapSwith(ScopedHandle& other) noexcept { swap(handle_, other.handle_); }

  bool IsValid() const { return handle_ != INVALID_HANDLE_VALUE; }

  // May throw if closing handle fails.
  void Reset(HANDLE handle = INVALID_HANDLE_VALUE);

  ALWAYS_INLINE HANDLE get() const { return handle_; }

  // Transfers ownership away from this object.
  HANDLE release() WARN_UNUSED_RESULT { return exchange(handle_, INVALID_HANDLE_VALUE); }

 private:
  HANDLE handle_;

  static void CloseHandle(HANDLE handle);

  DISALLOW_COPY_AND_ASSIGN(ScopedHandle);
};

inline void ScopedHandle::Reset(HANDLE handle) {
  if (handle_ != handle) {
    if (IsValid())
      CloseHandle(handle_);
    handle_ = handle;
  }
}

} // namespace win
} // namespace stp

#endif // STP_BASE_WIN_SCOPEDHANDLE_H_
