// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Win/ScopedHandle.h"

#include "Base/Error/SystemException.h"

namespace stp {
namespace win {

void ScopedHandle::CloseHandle(HANDLE handle) {
  if (!::CloseHandle(handle))
    throw Exception::with(SystemException(lastWinErrorCode()), "closing a handle failed");
}

} // namespace win
} // namespace stp
