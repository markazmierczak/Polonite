// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_TIME_TIMETICKS_H_
#define STP_BASE_TIME_TIMETICKS_H_

#include "Base/Time/TimeBase.h"

namespace stp {

// Represents monotonically non-decreasing clock time.
class BASE_EXPORT TimeTicks : public TimeBase<TimeTicks> {
 public:
  TimeTicks() : TimeBase(0) {}

  // Platform-dependent tick count representing "right now." When
  // IsHighResolution() returns false, the resolution of the clock could be
  // as coarse as ~15.6ms. Otherwise, the resolution should be no worse than one
  // microsecond.
  static TimeTicks Now();

  #if OS(WIN)
  // Translates an absolute QPC timestamp into a TimeTicks value. The returned
  // value has the same origin as Now(). Do NOT attempt to use this if
  // IsHighResolution() returns false.
  static TimeTicks FromQPCValue(LONGLONG qpc_value);
  #endif

  #if OS(MAC)
  static TimeTicks FromMachAbsoluteTime(uint64_t mach_absolute_time);
  #endif

  // Get an estimate of the TimeTick value at the time of the UnixEpoch. Because
  // Time and TimeTicks respond differently to user-set time and NTP
  // adjustments, this number is only an estimate. Nevertheless, this can be
  // useful when you need to relate the value of TimeTicks to a real time and
  // date. Note: Upon first invocation, this function takes a snapshot of the
  // realtime clock to establish a reference point.  This function will return
  // the same value for the duration of the application, but will be different
  // in future application runs.
  static TimeTicks UnixEpoch();

  // Returns |this| snapped to the next tick, given a |tick_phase| and
  // repeating |tick_interval| in both directions. |this| may be before,
  // after, or equal to the |tick_phase|.
  TimeTicks SnappedToNextTick(TimeTicks tick_phase, TimeDelta tick_interval) const;

  friend TextWriter& Format(TextWriter& out, TimeTicks x) {
    FormatImpl(out, x); return out;
  }
  friend void Format(TextWriter& out, TimeTicks x, const StringSpan& opts) {
    FormatImpl(out, x);
  }

 private:
  friend class TimeBase<TimeTicks>;
  friend class BaseApplicationPart;

  // Please use Now() to create a new object. This is for internal use and testing.
  explicit TimeTicks(int64_t us) : TimeBase(us) {}

  #if OS(WIN)
  static void ClassInit();
  #else
  static void ClassInit() {}
  #endif

  static void FormatImpl(TextWriter& out, TimeTicks x);
};

} // namespace stp

#endif // STP_BASE_TIME_TIMETICKS_H_
