// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Mem/RefCounted.h"

#include "Base/Test/GTest.h"

namespace stp {

namespace {
struct Object : RefCounted<Object> {};
} // namespace

static_assert(TIsZeroConstructible<RefPtr<Object>>, "!");
static_assert(TIsTriviallyRelocatable<RefPtr<Object>>, "!");
static_assert(TIsTriviallyEqualityComparable<RefPtr<Object>>, "!");

} // namespace stp
