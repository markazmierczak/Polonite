// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Debug/Alias.h"

#include "Base/Compiler/Config.h"

namespace stp {

#if COMPILER(MSVC)
#pragma optimize("", off)
#endif

void DebugAlias(const void* var) {
}

#if COMPILER(MSVC)
#pragma optimize("", on)
#endif

} // namespace stp
