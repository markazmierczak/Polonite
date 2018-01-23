// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_DEBUG_ALIAS_H_
#define STP_BASE_DEBUG_ALIAS_H_

#include "Base/Export.h"

namespace stp {

// Make the optimizer think that var is aliased. This is to prevent it from
// optimizing out variables that that would not otherwise be live at the point
// of a potential crash.
void BASE_EXPORT DebugAlias(const void* var);

} // namespace stp

#endif // STP_BASE_DEBUG_ALIAS_H_
