// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_TIME_TIMEDELTA_H_
#define STP_BASE_TIME_TIMEDELTA_H_

#include "Base/Compiler/Os.h"
#include "Base/Math/Safe.h"
#include "Base/Type/FormattableFwd.h"

#if OS(WIN)
#include "Base/Win/WindowsHeader.h"
#endif

extern "C" {
struct timespec;
}

namespace stp {

class BASE_EXPORT TimeDelta {
 public:
  static constexpr int64_t HoursPerDay = 24;
  static constexpr int64_t SecondsPerMinute = 60;
  static constexpr int64_t SecondsPerHour = SecondsPerMinute * 60;
  static constexpr int64_t SecondsPerDay = SecondsPerHour * HoursPerDay;
  static constexpr int64_t MillisecondsPerSecond = 1000;
  static constexpr int64_t MillisecondsPerDay = MillisecondsPerSecond * 60 * 60 * HoursPerDay;
  static constexpr int64_t MicrosecondsPerMillisecond = 1000;
  static constexpr int64_t MicrosecondsPerSecond = MicrosecondsPerMillisecond * MillisecondsPerSecond;
  static constexpr int64_t MicrosecondsPerMinute = MicrosecondsPerSecond * 60;
  static constexpr int64_t MicrosecondsPerHour = MicrosecondsPerMinute * 60;
  static constexpr int64_t MicrosecondsPerDay = MicrosecondsPerHour * HoursPerDay;
  static constexpr int64_t MicrosecondsPerWeek = MicrosecondsPerDay * 7;
  static constexpr int64_t NanosecondsPerMicrosecond = 1000;
  static constexpr int64_t NanosecondsPerSecond = NanosecondsPerMicrosecond * MicrosecondsPerSecond;

  constexpr TimeDelta() : delta_(0) {}

  // Converts units of time to TimeDeltas.
  static constexpr TimeDelta FromDays(int days);
  static constexpr TimeDelta FromHours(int hours);
  static constexpr TimeDelta FromMinutes(int minutes);
  static constexpr TimeDelta FromSeconds(int64_t secs);
  static constexpr TimeDelta FromMilliseconds(int64_t ms);
  static constexpr TimeDelta FromSecondsF(double secs);
  static constexpr TimeDelta FromMillisecondsF(double ms);
  static constexpr TimeDelta FromMicroseconds(int64_t us) { return TimeDelta(us); }

  #if OS(POSIX)
  static TimeDelta FromTimeSpec(const struct timespec& ts);
  struct timespec ToTimeSpec() const;
  #endif

  // Converts an integer value representing TimeDelta to a class. This is used
  // when deserializing a |TimeDelta| structure, using a value known to be
  // compatible. It is not provided as a constructor because the integer type
  // may be unclear from the perspective of a caller.
  static TimeDelta FromInternalValue(int64_t delta) { return TimeDelta(delta); }

  // Returns the internal numeric value of the TimeDelta object. Please don't
  // use this and do arithmetic on it, as it is more error prone than using the
  // provided operators.
  // For serializing, use FromInternalValue to reconstitute.
  int64_t ToInternalValue() const { return delta_; }

  TimeDelta GetMagnitude() const { return FromInternalValue(mathAbs(delta_)); }

  // Returns true if the time delta is zero.
  bool IsZero() const { return delta_ == 0; }

  // Returns the time delta in some unit. The F versions return a floating
  // point value, the "regular" versions return a rounded-down value.
  //
  // InMillisecondsRoundedUp() instead returns an integer that is rounded up
  // to the next full millisecond.
  constexpr int InDays() const;
  constexpr int InHours() const;
  constexpr int InMinutes() const;
  constexpr double InSecondsF() const;
  constexpr int64_t InSeconds() const;
  constexpr double InMillisecondsF() const;
  constexpr int64_t InMilliseconds() const;
  constexpr int64_t InMillisecondsRoundedUp() const;
  ALWAYS_INLINE constexpr int64_t InMicroseconds() const { return delta_; }

  // Computations with other deltas.
  TimeDelta operator+(TimeDelta other) const {
    return FromInternalValue(makeSafe(delta_) + other.delta_);
  }
  TimeDelta operator-(TimeDelta other) const {
    return FromInternalValue(makeSafe(delta_) - other.delta_);
  }

  TimeDelta& operator+=(TimeDelta other) { return *this = (*this + other); }
  TimeDelta& operator-=(TimeDelta other) { return *this = (*this - other); }

  TimeDelta operator-() const { return TimeDelta(-delta_); }

  // Computations with numeric types.
  template<typename T>
  TimeDelta operator*(T a) const { return FromInternalValue(makeSafe(delta_) * a); }
  template<typename T>
  TimeDelta operator/(T a) const { return FromInternalValue(makeSafe(delta_) / a); }

  template<typename T>
  TimeDelta& operator*=(T a) { return *this = (*this * a); }
  template<typename T>
  TimeDelta& operator/=(T a) { return *this = (*this / a); }

  int64_t operator/(TimeDelta a) const { return delta_ / a.delta_; }

  TimeDelta operator%(TimeDelta a) const { return TimeDelta(delta_ % a.delta_); }

  // Comparison operators.
  bool operator==(TimeDelta other) const { return delta_ == other.delta_; }
  bool operator!=(TimeDelta other) const { return delta_ != other.delta_; }
  bool operator< (TimeDelta other) const { return delta_ <  other.delta_; }
  bool operator<=(TimeDelta other) const { return delta_ <= other.delta_; }
  bool operator> (TimeDelta other) const { return delta_ >  other.delta_; }
  bool operator>=(TimeDelta other) const { return delta_ >= other.delta_; }

  friend TimeDelta mathAbs(TimeDelta x) { return x.GetMagnitude(); }

 private:
  // Constructs a delta given the duration in microseconds. This is private
  // to avoid confusion by callers with an integer constructor.
  // Use FromSeconds, FromMilliseconds, etc. instead.
  constexpr explicit TimeDelta(int64_t delta_us) : delta_(delta_us) {}

  // Private method to build a delta from a double.
  static constexpr TimeDelta FromDouble(double value);

  // Delta in microseconds.
  int64_t delta_;
};

template<typename T>
inline TimeDelta operator*(T a, TimeDelta td) {
  return td * a;
}

constexpr TimeDelta TimeDelta::FromDays(int days) {
  TimeDelta result(days * MicrosecondsPerDay);
  ASSERT(result.InDays() == days);
  return result;
}

constexpr TimeDelta TimeDelta::FromHours(int hours) {
  TimeDelta result(hours * MicrosecondsPerHour);
  ASSERT(result.InHours() == hours);
  return result;
}

constexpr TimeDelta TimeDelta::FromMinutes(int minutes) {
  TimeDelta result(minutes * MicrosecondsPerMinute);
  ASSERT(result.InMinutes() == minutes);
  return result;
}

constexpr TimeDelta TimeDelta::FromSeconds(int64_t secs) {
  TimeDelta result(secs * MicrosecondsPerSecond);
  ASSERT(result.InSeconds() == secs);
  return result;
}

constexpr TimeDelta TimeDelta::FromMilliseconds(int64_t ms) {
  TimeDelta result(ms * MicrosecondsPerMillisecond);
  ASSERT(result.InMilliseconds() == ms);
  return result;
}

constexpr TimeDelta TimeDelta::FromSecondsF(double secs) {
  return FromDouble(secs * MicrosecondsPerSecond);
}

constexpr TimeDelta TimeDelta::FromMillisecondsF(double ms) {
  return FromDouble(ms * MicrosecondsPerMillisecond);
}

constexpr TimeDelta TimeDelta::FromDouble(double value) {
  return TimeDelta(assertedCast<int64_t>(value));
}

constexpr int TimeDelta::InDays() const {
  return static_cast<int>(delta_ / MicrosecondsPerDay);
}

constexpr int TimeDelta::InHours() const {
  return static_cast<int>(delta_ / MicrosecondsPerHour);
}

constexpr int TimeDelta::InMinutes() const {
  return static_cast<int>(delta_ / MicrosecondsPerMinute);
}

constexpr double TimeDelta::InSecondsF() const {
  return static_cast<double>(delta_) / MicrosecondsPerSecond;
}

constexpr int64_t TimeDelta::InSeconds() const {
  return delta_ / MicrosecondsPerSecond;
}

constexpr double TimeDelta::InMillisecondsF() const {
  return static_cast<double>(delta_) / MicrosecondsPerMillisecond;
}

constexpr int64_t TimeDelta::InMilliseconds() const {
  return delta_ / MicrosecondsPerMillisecond;
}

constexpr int64_t TimeDelta::InMillisecondsRoundedUp() const {
  return (delta_ + MicrosecondsPerMillisecond - 1) / MicrosecondsPerMillisecond;
}

#if OS(WIN)

// Windows uses a Gregorian epoch of 1601. We need to match this internally
// so that our time representations match across all platforms.
//   irb(main):010:0> Time.at(0).getutc()
//   => Thu Jan 01 00:00:00 UTC 1970
//   irb(main):011:0> Time.at(-11644473600).getutc()
//   => Mon Jan 01 00:00:00 UTC 1601
constexpr int64_t WindowsEpochDeltaSeconds = INT64_C(11644473600);

// This value is the delta from the Windows epoch of 1601 to the POSIX delta of 1970.
constexpr int64_t WindowsEpochDeltaMicroseconds = WindowsEpochDeltaSeconds * TimeDelta::MicrosecondsPerSecond;

#endif // OS(WIN)

} // namespace stp

#endif // STP_BASE_TIME_TIMEDELTA_H_
