// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_TEST_SCOPEDLOCALE_H_
#define STP_BASE_TEST_SCOPEDLOCALE_H_

#include "Base/Containers/List.h"

namespace stp {

// Sets the given |locale| on construction, and restores the previous locale
// on destruction.
class ScopedLocale {
 public:
  explicit ScopedLocale(const char* locale);
  ~ScopedLocale();

 private:
  String prev_locale_;

  DISALLOW_COPY_AND_ASSIGN(ScopedLocale);
};

} // namespace stp

#endif // STP_BASE_TEST_SCOPEDLOCALE_H_
