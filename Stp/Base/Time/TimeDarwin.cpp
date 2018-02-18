// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Time/Time.h"

#include "Base/mac/mach_logging.h"
#include "Base/mac/scoped_cftyperef.h"
#include "Base/mac/scoped_mach_port.h"
#include "Base/Math/SafeConversions.h"
#include "Base/Time/ThreadTicks.h"
#include "Base/Time/TimeTicks.h"

#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFTimeZone.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

namespace stp {

namespace {

#if OS(MAC)
int64_t MachAbsoluteTimeToTicks(uint64_t mach_absolute_time) {
  static mach_timebase_info_data_t timebase_info;
  if (timebase_info.denom == 0) {
    // Zero-initialization of statics guarantees that denom will be 0 before
    // calling mach_timebase_info.  mach_timebase_info will never set denom to
    // 0 as that would be invalid, so the zero-check can be used to determine
    // whether mach_timebase_info has already been called.  This is
    // recommended by Apple's QA1398.
    kern_return_t kr = mach_timebase_info(&timebase_info);
    MACH_DCHECK(kr == KERN_SUCCESS, kr) << "mach_timebase_info";
  }

  // timebase_info converts absolute time tick units into nanoseconds.  Convert
  // to microseconds up front to stave off overflows.
  CheckedNumeric<uint64_t> result(mach_absolute_time / TimeDelta::NanosecondsPerMicrosecond);
  result *= timebase_info.numer;
  result /= timebase_info.denom;

  // Don't bother with the rollover handling that the Windows version does.
  // With numer and denom = 1 (the expected case), the 64-bit absolute time
  // reported in nanoseconds is enough to last nearly 585 years.
  return AssertedCast<int64_t>(result.ValueOrDie());
}
#endif // OS(MAC)

int64_t ComputeCurrentTicks() {
#if OS(IOS)
  // On iOS mach_absolute_time stops while the device is sleeping. Instead use
  // now - KERN_BOOTTIME to get a time difference that is not impacted by clock
  // changes. KERN_BOOTTIME will be updated by the system whenever the system
  // clock change.
  struct timeval boottime;
  int mib[2] = {CTL_KERN, KERN_BOOTTIME};
  size_t size = sizeof(boottime);
  int kr = sysctl(mib, isizeofArray(mib), &boottime, &size, nullptr, 0);
  ASSERT(kr == KERN_SUCCESS);
  TimeDelta time_difference =
      Time::Now() - (Time::FromTimeT(boottime.tv_sec) +
                           TimeDelta::FromMicroseconds(boottime.tv_usec));
  return time_difference.InMicroseconds();
#else
  // mach_absolute_time is it when it comes to ticks on the Mac.  Other calls
  // with less precision (such as TickCount) just call through to
  // mach_absolute_time.
  return MachAbsoluteTimeToTicks(mach_absolute_time());
#endif // OS(IOS)
}

int64_t ComputeThreadTicks() {
#if OS(IOS)
  ASSERT(false);
  return 0;
#else
  mac::ScopedMachSendRight thread(mach_thread_self());
  mach_msg_type_number_t thread_info_count = THREAD_BASIC_INFO_COUNT;
  thread_basic_info_data_t thread_info_data;

  if (thread.get() == MACH_PORT_NULL) {
    LOGF("Failed to get mach_thread_self()");
    return 0;
  }

  kern_return_t kr = thread_info(
      thread.get(),
      THREAD_BASIC_INFO,
      reinterpret_cast<thread_info_t>(&thread_info_data),
      &thread_info_count);
  MACH_DCHECK(kr == KERN_SUCCESS, kr) << "thread_info";

  CheckedNumeric<int64_t> absolute_micros(
      thread_info_data.user_time.seconds +
      thread_info_data.system_time.seconds);
  absolute_micros *= TimeDelta::MicrosecondsPerSecond;
  absolute_micros += (thread_info_data.user_time.microseconds +
                      thread_info_data.system_time.microseconds);
  return absolute_micros.ValueOrDie();
#endif // OS(IOS)
}

} // namespace

// The Time routines in this file use Mach and CoreFoundation APIs, since the
// POSIX definition of time_t in Mac OS X wraps around after 2038--and
// there are already cookie expiration dates, etc., past that time out in
// the field.  Using CFDate prevents that problem, and using mach_absolute_time
// for TimeTicks gives us nice high-resolution interval timing.

// Time -----------------------------------------------------------------------

Time Time::Now() {
  return FromCFAbsoluteTime(CFAbsoluteTimeGetCurrent());
}

Time Time::FromCFAbsoluteTime(CFAbsoluteTime t) {
  if (t == 0)
    return Time();  // Consider 0 as a null Time.
  if (t == Limits<CFAbsoluteTime>::Infinity)
    return Max();
  return Time(static_cast<int64_t>(t + kCFAbsoluteTimeIntervalSince1970) * TimeDelta::MicrosecondsPerSecond);
}

CFAbsoluteTime Time::ToCFAbsoluteTime() const {
  if (is_null())
    return 0;  // Consider 0 as a null Time.
  if (is_max())
    return Limits<CFAbsoluteTime>::Infinity;
  return (static_cast<CFAbsoluteTime>(us_) / TimeDelta::MicrosecondsPerSecond) - kCFAbsoluteTimeIntervalSince1970;
}

bool Time::FromExploded(bool is_local, const Exploded& exploded, Time* time) {
  ScopedCFTypeRef<CFTimeZoneRef> time_zone(
      is_local
          ? CFTimeZoneCopySystem()
          : CFTimeZoneCreateWithTimeIntervalFromGMT(kCFAllocatorDefault, 0));
  ScopedCFTypeRef<CFCalendarRef> gregorian(CFCalendarCreateWithIdentifier(
      kCFAllocatorDefault, kCFGregorianCalendar));
  CFCalendarSetTimeZone(gregorian, time_zone);
  CFAbsoluteTime absolute_time;
  // 'S' is not defined in componentDesc in Apple documentation, but can be
  // found at http://www.opensource.apple.com/source/CF/CF-855.17/CFCalendar.c
  CFCalendarComposeAbsoluteTime(
      gregorian, &absolute_time, "yMdHmsS", exploded.year, exploded.month,
      exploded.day_of_month, exploded.hour, exploded.minute, exploded.second,
      exploded.millisecond);
  CFAbsoluteTime seconds = absolute_time + kCFAbsoluteTimeIntervalSince1970;

  Time converted_time = Time(seconds * TimeDelta::MicrosecondsPerSecond);

  // If |exploded.day_of_month| is set to 31
  // on a 28-30 day month, it will return the first day of the next month.
  // Thus round-trip the time and compare the initial |exploded| with
  // |utc_to_exploded| time.
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

void Time::Explode(bool is_local, Exploded* exploded) const {
  // Avoid rounding issues, by only putting the integral number of seconds
  // (rounded towards -infinity) into a |CFAbsoluteTime| (which is a |double|).
  int64_t microsecond = us_ % TimeDelta::MicrosecondsPerSecond;
  if (microsecond < 0)
    microsecond += TimeDelta::MicrosecondsPerSecond;
  CFAbsoluteTime seconds = ((us_ - microsecond) / TimeDelta::MicrosecondsPerSecond) -
                           kCFAbsoluteTimeIntervalSince1970;

  ScopedCFTypeRef<CFTimeZoneRef> time_zone(
      is_local
          ? CFTimeZoneCopySystem()
          : CFTimeZoneCreateWithTimeIntervalFromGMT(kCFAllocatorDefault, 0));
  ScopedCFTypeRef<CFCalendarRef> gregorian(CFCalendarCreateWithIdentifier(
      kCFAllocatorDefault, kCFGregorianCalendar));
  CFCalendarSetTimeZone(gregorian, time_zone);
  int second, day_of_week;
  // 'E' sets the day of week, but is not defined in componentDesc in Apple
  // documentation. It can be found in open source code here:
  // http://www.opensource.apple.com/source/CF/CF-855.17/CFCalendar.c
  CFCalendarDecomposeAbsoluteTime(gregorian, seconds, "yMdHmsE",
                                  &exploded->year, &exploded->month,
                                  &exploded->day_of_month, &exploded->hour,
                                  &exploded->minute, &second, &day_of_week);
  // Make sure seconds are rounded down towards -infinity.
  exploded->second = floor(second);
  // |Exploded|'s convention for day of week is 0 = Sunday, i.e. different
  // from CF's 1 = Sunday.
  exploded->day_of_week = (day_of_week - 1) % 7;
  // Calculate milliseconds ourselves, since we rounded the |seconds|, making
  // sure to round towards -infinity.
  exploded->millisecond =
      (microsecond >= 0) ? microsecond / TimeDelta::MicrosecondsPerMillisecond
                         : (microsecond - TimeDelta::MicrosecondsPerMillisecond + 1) / TimeDelta::MicrosecondsPerMillisecond;
}

// TimeTicks ------------------------------------------------------------------

TimeTicks TimeTicks::Now() {
  return TimeTicks(ComputeCurrentTicks());
}

#if OS(MAC)
TimeTicks TimeTicks::FromMachAbsoluteTime(uint64_t mach_absolute_time) {
  return TimeTicks(MachAbsoluteTimeToTicks(mach_absolute_time));
}
#endif // OS(MAC)

ThreadTicks ThreadTicks::Now() {
  return ThreadTicks(ComputeThreadTicks());
}

} // namespace stp
