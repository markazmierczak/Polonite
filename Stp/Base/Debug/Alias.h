// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_DEBUG_ALIAS_H_
#define STP_BASE_DEBUG_ALIAS_H_

#include "Base/Compiler/Config.h"

namespace stp {

// Make the optimizer think that var is aliased. This is to prevent it from
// optimizing out variables that that would not otherwise be live at the point
// of a potential crash.
BASE_EXPORT void debugAlias(const void* var);

} // namespace stp

#endif // STP_BASE_DEBUG_ALIAS_H_
