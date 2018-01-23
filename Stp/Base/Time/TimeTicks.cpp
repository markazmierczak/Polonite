// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Time/TimeTicks.h"

#include "Base/Io/TextWriter.h"
#include "Base/Time/Time.h"
#include "Base/Type/Formattable.h"

namespace stp {

TimeTicks TimeTicks::UnixEpoch() {
  return TimeTicks::Now() - (Time::Now() - Time::UnixEpoch());
}

TimeTicks TimeTicks::SnappedToNextTick(TimeTicks tick_phase, TimeDelta tick_interval) const {
  // |interval_offset| is the offset from |this| to the next multiple of
  // |tick_interval| after |tick_phase|, possibly negative if in the past.
  TimeDelta interval_offset = (tick_phase - *this) % tick_interval;
  // If |this| is exactly on the interval (i.e. offset==0), don't adjust.
  // Otherwise, if |tick_phase| was in the past, adjust forward to the next
  // tick after |this|.
  if (!interval_offset.IsZero() && tick_phase < *this)
    interval_offset += tick_interval;
  return *this + interval_offset;
}

void TimeTicks::ToFormat(TextWriter& out, const StringSpan& opts) const {
  // This function formats a TimeTicks object as "bogo-microseconds".
  // The origin and granularity of the count are platform-specific, and may very
  // from run to run. Although bogo-microseconds usually roughly correspond to
  // real microseconds, the only real guarantee is that the number never goes
  // down during a single run.
  out << us_;
  out.WriteAscii(" bogo-microseconds");
}

} // namespace stp
