// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Memory/WeakPtr.h"

#include "Base/Compiler/Lsan.h"

namespace stp {
namespace detail {

WeakReference::Flag* WeakReference::Flag::Null = nullptr;

void WeakReference::Flag::classInit() {
  ANNOTATE_SCOPED_MEMORY_LEAK;
  Null = adoptRef(new Flag(0)).release();
}

} // namespace detail
} // namespace stp
