// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Time represents an absolute point in coordinated universal time (UTC),
// internally represented as microseconds since January 1, 1970, 0:00.
// System-dependent clock interface routines are defined in TimePlatform.cpp.
// Note that values for Time may skew and jump around as the operating system
// makes adjustments to synchronize (e.g., with NTP servers).
// Thus, client code that uses the Time class must account for this.
//
// TimeDelta represents a duration of time, internally represented in microseconds.
//
// TimeTicks and ThreadTicks represent an abstract time that is most of the time
// incrementing, for use in measuring time durations. Internally, they are
// represented in microseconds. They can not be converted to a human-readable
// time, but are guaranteed not to decrease (unlike the Time class). Note that
// TimeTicks may "stand still" (e.g., if the computer is suspended), and
// ThreadTicks will "stand still" whenever the thread has been de-scheduled by
// the operating system.
//
// All time classes are copyable, assignable, and occupy 64-bits per instance.
// As a result, prefer passing them by value:
//   void MyFunction(TimeDelta arg);
// If circumstances require, you may also pass by const reference:
//   void MyFunction(const TimeDelta& arg);  // Not preferred.
//
// So many choices!  Which time class should you use?  Examples:
//
//   Time:        Interpreting the wall-clock time provided by a remote
//                system. Detecting whether cached resources have
//                expired. Providing the user with a display of the current date
//                and time. Determining the amount of time between events across
//                re-boots of the machine.
//
//   TimeTicks:   Tracking the amount of time a task runs. Executing delayed
//                tasks at the right time. Computing presentation timestamps.
//                Synchronizing audio and video using TimeTicks as a common
//                reference clock (lip-sync). Measuring network round-trip
//                latency.
//
//   ThreadTicks: Benchmarking how long the current thread has been doing actual work.

#ifndef STP_BASE_TIME_TIME_H_
#define STP_BASE_TIME_TIME_H_

#include "Base/Time/TimeBase.h"

#include <time.h>

#if OS(DARWIN)
#include <CoreFoundation/CoreFoundation.h>
// Avoid Mac system header macro leak.
#undef TYPE_BOOL
#endif

#if OS(POSIX)
#include <unistd.h>
#include <sys/time.h>
#endif

#if OS(WIN)
extern "C" {
struct _FILETIME;
}
#endif

namespace stp {

// Represents a wall clock time in UTC. Values are not guaranteed to be
// monotonically non-decreasing and are subject to large amounts of skew.
class BASE_EXPORT Time : public TimeBase<Time> {
 public:
  // Represents an exploded time that can be formatted nicely. This is kind of
  // like the Win32 SYSTEMTIME structure or the Unix "struct tm" with a few
  // additions and changes to prevent errors.
  struct BASE_EXPORT Exploded {
    int year;          // Four digit year "2007"
    int month;         // 1-based month (values 1 = January, etc.)
    int day_of_week;   // 0-based day of week (0 = Sunday, etc.)
    int day_of_month;  // 1-based day of month (1-31)
    int hour;          // Hour within the current day (0-23)
    int minute;        // Minute within the current hour (0-59)
    int second;        // Second within the current minute (0-59 plus leap
                       //   seconds which may take it up to 60).
    int millisecond;   // Milliseconds within the current second (0-999)

    // A cursory test for whether the data members are within their
    // respective ranges. A 'true' return value does not guarantee the
    // Exploded value can be successfully converted to a Time value.
    bool HasValidValues() const;
  };

  // Contains the null time. Use Time::Now() to get the current time.
  Time() : TimeBase(0) {}

  // Returns the time for epoch in Unix-like system (Jan 1, 1970).
  static Time UnixEpoch() { return Time(0); }

  // Returns the current time. Watch out, the system might adjust its clock
  // in which case time will actually go backwards. We don't guarantee that
  // times are increasing, or that two calls to Now() won't be the same.
  static Time Now();

  // Converts to/from time_t in UTC and a Time class.
  static Time FromTimeT(time_t tt);
  time_t ToTimeT() const;

  // Converts time to/from a double which is the number of seconds since epoch
  // (Jan 1, 1970).  Webkit uses this format to represent time.
  // Because WebKit initializes double time value to 0 to indicate "not
  // initialized", we map it to empty Time object that also means "not
  // initialized".
  static Time FromDoubleT(double dt);
  double ToDoubleT() const;

  #if OS(POSIX)
  // Converts the timespec structure to time. MacOS X 10.8.3 (and tentatively,
  // earlier versions) will have the |ts|'s tv_nsec component zeroed out,
  // having a 1 second resolution, which agrees with
  // https://developer.apple.com/legacy/library/#technotes/tn/tn1150.html#HFSPlusDates.
  static Time FromTimeSpec(timespec ts);
  timespec ToTimeSpec() const;
  #endif

  // Converts to/from the Javascript convention for times, a number of
  // milliseconds since the epoch:
  // https://developer.mozilla.org/en/JavaScript/Reference/Global_Objects/Date/getTime.
  static Time FromJSTime(double ms_since_epoch);
  double ToJSTime() const;

  // Converts from/to Java convention for times, a number of milliseconds since the epoch.
  static Time FromJavaTime(int64_t ms_since_epoch);
  int64_t ToJavaTime() const;

  #if OS(POSIX)
  static Time FromTimeVal(struct timeval t);
  struct timeval ToTimeVal() const;
  #endif

  #if OS(DARWIN)
  static Time FromCFAbsoluteTime(CFAbsoluteTime t);
  CFAbsoluteTime ToCFAbsoluteTime() const;
  #endif

  #if OS(WIN)
  static Time FromFileTime(struct _FILETIME ft);
  struct _FILETIME ToFileTime() const;

  // The minimum time of a low resolution timer.  This is basically a windows
  // constant of ~15.6ms.  While it does vary on some older OS versions, we'll
  // treat it as static across all windows versions.
  static const int kMinLowResolutionThresholdMs = 16;

  // Activates or deactivates the high resolution timer based on the |activate| flag.
  // Each successful activate call must be paired with a subsequent deactivate call.
  // All callers to activate the high resolution timer must eventually call
  // this function to deactivate the high resolution timer.
  static void ActivateHighResolutionTimer(bool activate);

  // Returns true if the high resolution timer is both activated.
  static bool IsHighResolutionTimerInUse();
  #endif

  // Converts an exploded structure representing either the local time or UTC
  // into a Time class. Returns false on a failure when, for example, a day of
  // month is set to 31 on a 28-30 day month.
  static bool FromUTCExploded(const Exploded& exploded, Time* time) WARN_UNUSED_RESULT {
    return FromExploded(false, exploded, time);
  }
  static bool FromLocalExploded(const Exploded& exploded, Time* time) WARN_UNUSED_RESULT {
    return FromExploded(true, exploded, time);
  }

  // Converts a string representation of time to a Time object.
  // An example of a time string which is converted is as below:-
  // "Tue, 15 Nov 1994 12:45:26 GMT". If the timezone is not specified
  // in the input string, FromString assumes local time and FromUTCString
  // assumes UTC. A timezone that cannot be parsed (e.g. "UTC" which is not
  // specified in RFC822) is treated as if the timezone is not specified.
  static bool FromString(const char* time_string, Time* parsed_time) WARN_UNUSED_RESULT {
    return FromStringInternal(time_string, true, parsed_time);
  }
  static bool FromUTCString(const char* time_string, Time* parsed_time) WARN_UNUSED_RESULT {
    return FromStringInternal(time_string, false, parsed_time);
  }

  // Fills the given exploded structure with either the local time or UTC from
  // this time structure (containing UTC).
  void UTCExplode(Exploded* exploded) const { Explode(false, exploded); }
  void LocalExplode(Exploded* exploded) const { Explode(true, exploded); }

  // Rounds this time down to the nearest day in local time. It will represent
  // midnight on that day.
  Time LocalMidnight() const;

  friend TextWriter& operator<<(TextWriter& out, Time x) {
    formatImpl(out, x); return out;
  }
  friend void format(TextWriter& out, Time x, const StringSpan& opts) {
    formatImpl(out, x);
  }

 private:
  friend class TimeBase<Time>;

  explicit Time(int64_t us) : TimeBase(us) {}

  // Explodes the given time to either local time |is_local = true| or UTC
  // |is_local = false|.
  void Explode(bool is_local, Exploded* exploded) const;

  // Unexplodes a given time assuming the source is either local time
  // |is_local = true| or UTC |is_local = false|. Function returns false on
  // failure and sets |time| to Time(0). Otherwise returns true and sets |time|
  // to non-exploded time.
  static bool FromExploded(
      bool is_local,
      const Exploded& exploded,
      Time* time) WARN_UNUSED_RESULT;

  // Converts a string representation of time to a Time object.
  // An example of a time string which is converted is as below:-
  // "Tue, 15 Nov 1994 12:45:26 GMT". If the timezone is not specified
  // in the input string, local time |is_local = true| or
  // UTC |is_local = false| is assumed. A timezone that cannot be parsed
  // (e.g. "UTC" which is not specified in RFC822) is treated as if the
  // timezone is not specified.
  static bool FromStringInternal(
      const char* time_string,
      bool is_local,
      Time* parsed_time);

  // Comparison does not consider |day_of_week| when doing the operation.
  static bool ExplodedMostlyEquals(const Exploded& lhs, const Exploded& rhs);

  static void formatImpl(TextWriter& out, Time x);
};

} // namespace stp

#endif // STP_BASE_TIME_TIME_H_
