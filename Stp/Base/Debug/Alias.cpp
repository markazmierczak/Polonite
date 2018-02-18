// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/Alias.h"

#include "Base/Compiler/Config.h"

namespace stp {

#if COMPILER(MSVC)
#pragma optimize("", off)
#endif

void debugAlias(const void* var) {
}

#if COMPILER(MSVC)
#pragma optimize("", on)
#endif

} // namespace stp
