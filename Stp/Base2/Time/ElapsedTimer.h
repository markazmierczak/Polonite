// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_TIME_ELAPSEDTIMER_H_
#define STP_BASE_TIME_ELAPSEDTIMER_H_

#include "Base/Time/TimeTicks.h"

namespace stp {

// A simple wrapper around TimeTicks::Now().
class ElapsedTimer {
 public:
  ElapsedTimer() { begin_ = TimeTicks::Now(); }

  // Returns the time elapsed since object construction.
  TimeDelta Elapsed() const { return TimeTicks::Now() - begin_; }

 private:
  TimeTicks begin_;
};

} // namespace stp

#endif // STP_BASE_TIME_ELAPSEDTIMER_H_
