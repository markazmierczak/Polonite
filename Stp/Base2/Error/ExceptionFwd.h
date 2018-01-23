// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_ERROR_EXCEPTIONFWD_H_
#define STP_BASE_ERROR_EXCEPTIONFWD_H_

#include "Base/Export.h"

namespace stp {

class Exception;

BASE_EXPORT int CountUncaughtExceptions() noexcept;
BASE_EXPORT bool HasUncaughtExceptions() noexcept;

} // namespace stp

#endif // STP_BASE_ERROR_EXCEPTIONFWD_H_
