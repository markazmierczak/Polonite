// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Time/Time.h"

#include "Base/Debug/Log.h"
#include "Base/Text/FormatMany.h"
#include "Base/Time/ThreadTicks.h"
#include "Base/Time/TimeTicks.h"

#include <sys/time.h>
#include <unistd.h>

#if OS(ANDROID) && !defined(__LP64__)
#include <time64.h>
#endif

#if OS(ANDROID)
#include "Base/Android/OsCompatAndroid.h"
#endif

#if !OS(DARWIN)
#include "Base/Thread/Lock.h"
#include "Base/Util/LazyInstance.h"
#endif

namespace stp {

namespace {

#if !OS(DARWIN)
// This prevents a crash on traversing the environment global and looking up
// the 'TZ' variable in libc.
LazyInstance<Lock>::LeakAtExit g_sys_time_to_time_struct_lock = LAZY_INSTANCE_INITIALIZER;

// Define a system-specific SysTime that wraps either to a time_t or
// a time64_t depending on the host system, and associated conversion.
#if OS(ANDROID) && !defined(__LP64__)
typedef time64_t SysTime;

SysTime SysTimeFromTimeStruct(struct tm* timestruct, bool is_local) {
  AutoLock locked(&g_sys_time_to_time_struct_lock.get());
  if (is_local)
    return mktime64(timestruct);
  else
    return timegm64(timestruct);
}

void SysTimeToTimeStruct(SysTime t, struct tm* timestruct, bool is_local) {
  AutoLock locked(&g_sys_time_to_time_struct_lock.get());
  if (is_local)
    localtime64_r(&t, timestruct);
  else
    gmtime64_r(&t, timestruct);
}

#else
typedef time_t SysTime;

SysTime SysTimeFromTimeStruct(struct tm* timestruct, bool is_local) {
  AutoLock locked(&*g_sys_time_to_time_struct_lock);
  if (is_local)
    return mktime(timestruct);
  else
    return timegm(timestruct);
}

void SysTimeToTimeStruct(SysTime t, struct tm* timestruct, bool is_local) {
  AutoLock locked(&*g_sys_time_to_time_struct_lock);
  if (is_local)
    localtime_r(&t, timestruct);
  else
    gmtime_r(&t, timestruct);
}
#endif // OS(ANDROID) && !__LP64__

int64_t ConvertTimespecToMicros(const struct timespec& ts) {
  // On 32-bit systems, the calculation cannot overflow int64_t.
  // 2**32 * 1000000 + 2**64 / 1000 < 2**63
  if (sizeof(ts.tv_sec) <= 4 && sizeof(ts.tv_nsec) <= 8) {
    int64_t result = ts.tv_sec;
    result *= TimeDelta::MicrosecondsPerSecond;
    result += (ts.tv_nsec / TimeDelta::NanosecondsPerMicrosecond);
    return result;
  }
  auto msecs_from_secs = MakeSafe(ts.tv_sec) * TimeDelta::MicrosecondsPerSecond;
  return msecs_from_secs + ts.tv_nsec / TimeDelta::NanosecondsPerMicrosecond;
}

// Helper function to get results from clock_gettime() and convert to a
// microsecond timebase. Minimum requirement is MONOTONIC_CLOCK to be supported
// on the system. FreeBSD 6 has CLOCK_MONOTONIC but defines
// _POSIX_MONOTONIC_CLOCK to -1.
#if (OS(POSIX) && defined(_POSIX_MONOTONIC_CLOCK) && _POSIX_MONOTONIC_CLOCK >= 0) || \
    OS(BSD) || OS(ANDROID)
int64_t ClockNow(clockid_t clk_id) {
  struct timespec ts;
  if (clock_gettime(clk_id, &ts) != 0) {
    ASSERT(false, "clock_gettime({}) failed", clk_id);
    return 0;
  }
  return ConvertTimespecToMicros(ts);
}
#else // _POSIX_MONOTONIC_CLOCK
#error "no usable tick clock function on this platform"
#endif // _POSIX_MONOTONIC_CLOCK
#endif // !OS(DARWIN)

} // namespace

TimeDelta TimeDelta::FromTimeSpec(const timespec& ts) {
  return TimeDelta(ts.tv_sec * TimeDelta::MicrosecondsPerSecond +
                   ts.tv_nsec / TimeDelta::NanosecondsPerMicrosecond);
}

struct timespec TimeDelta::ToTimeSpec() const {
  int64_t microseconds = InMicroseconds();
  time_t seconds = 0;
  if (microseconds >= TimeDelta::MicrosecondsPerSecond) {
    seconds = InSeconds();
    microseconds -= seconds * TimeDelta::MicrosecondsPerSecond;
  }
  struct timespec result =
      {seconds, static_cast<long>(microseconds * TimeDelta::NanosecondsPerMicrosecond)};
  return result;
}

#if !OS(DARWIN)
// The Time routines in this file use standard POSIX routines, or almost-
// standard routines in the case of timegm.  We need to use a Mach-specific
// function for TimeTicks::Now() on Mac OS X.

// Time -----------------------------------------------------------------------

Time Time::FromTimeSpec(timespec ts) {
  return Time() + TimeDelta::FromTimeSpec(ts);
}

timespec Time::ToTimeSpec() const {
  return (*this - Time()).ToTimeSpec();
}

Time Time::Now() {
  struct timeval tv;
  struct timezone tz = { 0, 0 };  // UTC
  if (gettimeofday(&tv, &tz) != 0) {
    RELEASE_LOG(ERROR, "gettimeofday failed");
    ASSERT(false, "could not determine time of day");
    // Return null instead of uninitialized |tv| value, which contains random garbage data.
    return Time();
  }
  // Combine seconds and microseconds in a 64-bit field containing microseconds since the epoch.
  return Time(tv.tv_sec * TimeDelta::MicrosecondsPerSecond + tv.tv_usec);
}

void Time::Explode(bool is_local, Exploded* exploded) const {
  // Time stores times with microsecond resolution, but Exploded only carries
  // millisecond resolution, so begin by being lossy.
  int64_t microseconds = us_;
  // The following values are all rounded towards -infinity.
  int64_t milliseconds;  // Milliseconds since epoch.
  SysTime seconds;  // Seconds since epoch.
  int millisecond;  // Exploded millisecond value (0-999).
  if (microseconds >= 0) {
    // Rounding towards -infinity <=> rounding towards 0, in this case.
    milliseconds = microseconds / TimeDelta::MicrosecondsPerMillisecond;
    seconds = milliseconds / TimeDelta::MillisecondsPerSecond;
    millisecond = milliseconds % TimeDelta::MillisecondsPerSecond;
  } else {
    // Round these *down* (towards -infinity).
    milliseconds = (microseconds - TimeDelta::MicrosecondsPerMillisecond + 1) / TimeDelta::MicrosecondsPerMillisecond;
    seconds = (milliseconds - TimeDelta::MillisecondsPerSecond + 1) / TimeDelta::MillisecondsPerSecond;
    // Make this nonnegative (and between 0 and 999 inclusive).
    millisecond = milliseconds % TimeDelta::MillisecondsPerSecond;
    if (millisecond < 0)
      millisecond += TimeDelta::MillisecondsPerSecond;
  }

  struct tm timestruct;
  SysTimeToTimeStruct(seconds, &timestruct, is_local);

  exploded->year         = timestruct.tm_year + 1900;
  exploded->month        = timestruct.tm_mon + 1;
  exploded->day_of_week  = timestruct.tm_wday;
  exploded->day_of_month = timestruct.tm_mday;
  exploded->hour         = timestruct.tm_hour;
  exploded->minute       = timestruct.tm_min;
  exploded->second       = timestruct.tm_sec;
  exploded->millisecond  = millisecond;
}

bool Time::FromExploded(bool is_local, const Exploded& exploded, Time* time) {
  struct tm timestruct;
  timestruct.tm_sec    = exploded.second;
  timestruct.tm_min    = exploded.minute;
  timestruct.tm_hour   = exploded.hour;
  timestruct.tm_mday   = exploded.day_of_month;
  timestruct.tm_mon    = exploded.month - 1;
  timestruct.tm_year   = exploded.year - 1900;
  timestruct.tm_wday   = exploded.day_of_week;  // mktime/timegm ignore this
  timestruct.tm_yday   = 0;     // mktime/timegm ignore this
  timestruct.tm_isdst  = -1;    // attempt to figure it out
  timestruct.tm_gmtoff = 0;     // not a POSIX field, so mktime/timegm ignore
  timestruct.tm_zone   = nullptr; // not a POSIX field, so mktime/timegm ignore

  int64_t milliseconds;
  SysTime seconds;

  // Certain exploded dates do not really exist due to daylight saving times,
  // and this causes mktime() to return implementation-defined values when
  // tm_isdst is set to -1. On Android, the function will return -1, while the
  // C libraries of other platforms typically return a liberally-chosen value.
  // Handling this requires the special code below.

  // SysTimeFromTimeStruct() modifies the input structure, save current value.
  struct tm timestruct0 = timestruct;

  seconds = SysTimeFromTimeStruct(&timestruct, is_local);
  if (seconds == -1) {
    // Get the time values with tm_isdst == 0 and 1, then select the closest one
    // to UTC 00:00:00 that isn't -1.
    timestruct = timestruct0;
    timestruct.tm_isdst = 0;
    int64_t seconds_isdst0 = SysTimeFromTimeStruct(&timestruct, is_local);

    timestruct = timestruct0;
    timestruct.tm_isdst = 1;
    int64_t seconds_isdst1 = SysTimeFromTimeStruct(&timestruct, is_local);

    // seconds_isdst0 or seconds_isdst1 can be -1 for some timezones.
    // E.g. "CLST" (Chile Summer Time) returns -1 for 'tm_isdt == 1'.
    if (seconds_isdst0 < 0)
      seconds = seconds_isdst1;
    else if (seconds_isdst1 < 0)
      seconds = seconds_isdst0;
    else
      seconds = min(seconds_isdst0, seconds_isdst1);
  }

  // Handle overflow.  Clamping the range to what mktime and timegm might
  // return is the best that can be done here.  It's not ideal, but it's better
  // than failing here or ignoring the overflow case and treating each time
  // overflow as one second prior to the epoch.
  if (seconds == -1 &&
      (exploded.year < 1969 || exploded.year > 1970)) {
    // If exploded.year is 1969 or 1970, take -1 as correct, with the
    // time indicating 1 second prior to the epoch.  (1970 is allowed to handle
    // time zone and DST offsets.)  Otherwise, return the most future or past
    // time representable.  Assumes the time_t epoch is 1970-01-01 00:00:00 UTC.
    //
    // The minimum and maximum representible times that mktime and timegm could
    // return are used here instead of values outside that range to allow for
    // proper round-tripping between exploded and counter-type time
    // representations in the presence of possible truncation to time_t by
    // division and use with other functions that accept time_t.
    //
    // When representing the most distant time in the future, add in an extra
    // 999ms to avoid the time being less than any other possible value that
    // this function can return.

    // On Android, SysTime is int64_t, special care must be taken to avoid
    // overflows.
    const int64_t min_seconds = (sizeof(SysTime) < sizeof(int64_t))
                                    ? Limits<SysTime>::Min
                                    : Limits<int32_t>::Min;
    const int64_t max_seconds = (sizeof(SysTime) < sizeof(int64_t))
                                    ? Limits<SysTime>::Max
                                    : Limits<int32_t>::Max;
    if (exploded.year < 1969) {
      milliseconds = min_seconds * TimeDelta::MillisecondsPerSecond;
    } else {
      milliseconds = max_seconds * TimeDelta::MillisecondsPerSecond;
      milliseconds += (TimeDelta::MillisecondsPerSecond - 1);
    }
  } else {
    milliseconds = seconds * TimeDelta::MillisecondsPerSecond + exploded.millisecond;
  }

  Time converted_time = Time(milliseconds * TimeDelta::MicrosecondsPerMillisecond);

  // If |exploded.day_of_month| is set to 31 on a 28-30 day month, it will
  // return the first day of the next month. Thus round-trip the time and
  // compare the initial |exploded| with |utc_to_exploded| time.
  Time::Exploded to_exploded;
  if (!is_local)
    converted_time.UTCExplode(&to_exploded);
  else
    converted_time.LocalExplode(&to_exploded);

  if (ExplodedMostlyEquals(to_exploded, exploded)) {
    *time = converted_time;
    return true;
  }
  return false;
}

// TimeTicks ------------------------------------------------------------------
TimeTicks TimeTicks::Now() {
  return TimeTicks(ClockNow(CLOCK_MONOTONIC));
}

ThreadTicks ThreadTicks::Now() {
  #if (defined(_POSIX_THREAD_CPUTIME) && (_POSIX_THREAD_CPUTIME >= 0)) || OS(ANDROID)
  return ThreadTicks(ClockNow(CLOCK_THREAD_CPUTIME_ID));
  #else
  ASSERT(false);
  return ThreadTicks();
  #endif
}

#endif // !OS(DARWIN)

Time Time::FromTimeVal(struct timeval t) {
  ASSERT(0 <= t.tv_usec && t.tv_usec < static_cast<int>(TimeDelta::MicrosecondsPerSecond));
  return Time(static_cast<int64_t>(t.tv_sec) * TimeDelta::MicrosecondsPerSecond + t.tv_usec);
}

struct timeval Time::ToTimeVal() const {
  struct timeval result;
  int64_t us = us_;
  result.tv_sec = us / TimeDelta::MicrosecondsPerSecond;
  result.tv_usec = us % TimeDelta::MicrosecondsPerSecond;
  return result;
}

} // namespace stp
