// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Memory/RefCounted.h"

#include "Base/Memory/RcPtr.h"
#include "Base/Test/GTest.h"

namespace stp {

namespace {
struct Object : RefCounted<Object> {};
} // namespace

static_assert(TIsZeroConstructible<RcPtr<Object>>, "!");
static_assert(TIsTriviallyRelocatable<RcPtr<Object>>, "!");
static_assert(TIsTriviallyEqualityComparable<RcPtr<Object>>, "!");

} // namespace stp
