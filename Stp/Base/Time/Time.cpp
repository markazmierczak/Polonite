// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Time/Time.h"

#include <inttypes.h>

#include "Base/Math/Math.h"
#include "Base/Text/Format.h"
#include "Base/ThirdParty/nspr/prtime.h"
#include "Base/Time/ThreadTicks.h"
#include "Base/Type/Limits.h"

namespace stp {

Time Time::FromTimeT(time_t tt) {
  return Time::UnixEpoch() + TimeDelta::FromSeconds(tt);
}

time_t Time::ToTimeT() const {
  return AssertedCast<time_t>(us_ / TimeDelta::MicrosecondsPerSecond);
}

Time Time::FromDoubleT(double dt) {
  if (dt == 0 || IsNaN(dt))
    return Time();  // Preserve 0 so we can tell it doesn't exist.
  return UnixEpoch() + TimeDelta::FromSecondsF(dt);
}

double Time::ToDoubleT() const {
  return static_cast<double>(us_) / TimeDelta::MicrosecondsPerSecond;
}

Time Time::FromJSTime(double ms_since_epoch) {
  // The epoch is a valid time, so this constructor doesn't interpret
  // 0 as the null time.
  return UnixEpoch() + TimeDelta::FromMillisecondsF(ms_since_epoch);
}

double Time::ToJSTime() const {
  return static_cast<double>(us_) / TimeDelta::MicrosecondsPerMillisecond;
}

Time Time::FromJavaTime(int64_t ms_since_epoch) {
  return Time::UnixEpoch() + TimeDelta::FromMilliseconds(ms_since_epoch);
}

int64_t Time::ToJavaTime() const {
  return us_ / TimeDelta::MicrosecondsPerMillisecond;
}

Time Time::LocalMidnight() const {
  Exploded exploded;
  LocalExplode(&exploded);
  exploded.hour = 0;
  exploded.minute = 0;
  exploded.second = 0;
  exploded.millisecond = 0;

  Time result;
  bool ok = FromLocalExploded(exploded, &result);
  ASSERT_UNUSED(ok, ok);
  return result;
}

bool Time::FromStringInternal(const char* time_string, bool is_local, Time* parsed_time) {
  ASSERT(time_string && parsed_time);

  if (time_string[0] == '\0')
    return false;

  PRTime result_time = 0;
  PRStatus result = PR_ParseTimeString(
      time_string,
      is_local ? PR_FALSE : PR_TRUE,
          &result_time);
  if (PR_SUCCESS != result)
    return false;

  *parsed_time = Time(result_time);
  return true;
}

bool Time::ExplodedMostlyEquals(const Exploded& lhs, const Exploded& rhs) {
  return lhs.year == rhs.year && lhs.month == rhs.month &&
         lhs.day_of_month == rhs.day_of_month && lhs.hour == rhs.hour &&
         lhs.minute == rhs.minute && lhs.second == rhs.second &&
         lhs.millisecond == rhs.millisecond;
}

void Time::ToFormat(TextWriter& out, const StringSpan& opts) const {
  Exploded exploded;
  UTCExplode(&exploded);
  FormatMany(out,
      "{:04}-{:02}-{:02} "
      "{:02}:{:02}:{:02}.{:03} UTC",
      exploded.year, exploded.month, exploded.day_of_month,
      exploded.hour, exploded.minute, exploded.second, exploded.millisecond);
}

void ThreadTicks::ToFormat(TextWriter& out, const StringSpan& opts) const {
  out << us_;
  out.WriteAscii(" bogo-thread-microseconds");
}

// Time::Exploded -------------------------------------------------------------

inline bool IsInRange(int value, int lo, int hi) {
  return lo <= value && value <= hi;
}

bool Time::Exploded::HasValidValues() const {
  return IsInRange(month, 1, 12) &&
         IsInRange(day_of_week, 0, 6) &&
         IsInRange(day_of_month, 1, 31) &&
         IsInRange(hour, 0, 23) &&
         IsInRange(minute, 0, 59) &&
         IsInRange(second, 0, 60) &&
         IsInRange(millisecond, 0, 999);
}

} // namespace stp
