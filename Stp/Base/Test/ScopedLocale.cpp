// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Test/ScopedLocale.h"

#include "Base/Test/GTest.h"

#include <locale.h>

namespace stp {

ScopedLocale::ScopedLocale(const char* locale) {
  prev_locale_ = makeSpanFromNullTerminated(setlocale(LC_ALL, NULL));
  EXPECT_TRUE(setlocale(LC_ALL, locale) != NULL) << "Failed to set locale: " << locale;
}

ScopedLocale::~ScopedLocale() {
  auto* prev = toNullTerminated(prev_locale_);
  EXPECT_STREQ(prev, setlocale(LC_ALL, prev));
}

} // namespace stp
