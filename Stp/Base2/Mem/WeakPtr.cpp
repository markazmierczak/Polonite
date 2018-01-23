// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Mem/WeakPtr.h"

#include "Base/Compiler/Lsan.h"

namespace stp {
namespace detail {

WeakReference::Flag* WeakReference::Flag::Null = nullptr;

void WeakReference::Flag::ClassInit() {
  ANNOTATE_SCOPED_MEMORY_LEAK;
  Null = AdoptRef(new Flag(0)).Release();
}

} // namespace detail
} // namespace stp
