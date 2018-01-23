// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TIME_TIMEDELTAF_H_
#define STP_BASE_TIME_TIMEDELTAF_H_

#include "Base/Math/Math.h"
#include "Base/Time/TimeDelta.h"
#include "Base/Type/Limits.h"

namespace stp {

class TimeDeltaF {
 public:
  static constexpr double HoursPerDay = 24;
  static constexpr double SecondsPerMinute = 60;
  static constexpr double SecondsPerHour = SecondsPerMinute * 60;
  static constexpr double SecondsPerDay = SecondsPerHour * HoursPerDay;
  static constexpr double MillisecondsPerSecond = 1000;
  static constexpr double MillisecondsPerDay = MillisecondsPerSecond * 60 * 60 * HoursPerDay;
  static constexpr double MicrosecondsPerMillisecond = 1000;
  static constexpr double MicrosecondsPerSecond = MicrosecondsPerMillisecond * MillisecondsPerSecond;
  static constexpr double MicrosecondsPerMinute = MicrosecondsPerSecond * 60;
  static constexpr double MicrosecondsPerHour = MicrosecondsPerMinute * 60;
  static constexpr double MicrosecondsPerDay = MicrosecondsPerHour * HoursPerDay;
  static constexpr double MicrosecondsPerWeek = MicrosecondsPerDay * 7;
  static constexpr double NanosecondsPerMicrosecond = 1000;
  static constexpr double NanosecondsPerSecond = NanosecondsPerMicrosecond * MicrosecondsPerSecond;

  constexpr TimeDeltaF() : secs_(0) {}

  explicit TimeDeltaF(TimeDelta o)
      : secs_(static_cast<double>(o.InMicroseconds()) / MicrosecondsPerSecond) {}

  // Converts units of time to TimeDeltaF.
  static constexpr TimeDeltaF FromDaysF(double days) {
    return TimeDeltaF(days * SecondsPerDay);
  }
  static constexpr TimeDeltaF FromHoursF(double hours) {
    return TimeDeltaF(hours * SecondsPerHour);
  }
  static constexpr TimeDeltaF FromMinutesF(double minutes) {
    return TimeDeltaF(minutes * SecondsPerMinute);
  }
  static constexpr TimeDeltaF FromSecondsF(double secs) {
    return TimeDeltaF(secs);
  }
  static constexpr TimeDeltaF FromMillisecondsF(double ms) {
    return TimeDeltaF(ms / MillisecondsPerSecond);
  }
  static constexpr TimeDeltaF FromMicrosecondsF(double us) {
    return TimeDeltaF(us / MicrosecondsPerSecond);
  }

  // Made public for serialization only.
  static ALWAYS_INLINE TimeDeltaF FromInternalValue(double delta) { return TimeDeltaF(delta); }
  ALWAYS_INLINE double ToInternalValue() const { return secs_; }

  static TimeDeltaF Null() { return TimeDeltaF(Limits<double>::NaN); }

  TimeDeltaF GetMagnitude() const { return TimeDeltaF(Abs(secs_)); }

  // Returns true if the time delta is zero.
  bool IsZero() const { return secs_ == 0; }

  bool IsNull() const { return IsNaN(secs_) != 0; }

  // Returns the time delta in some unit.
  constexpr double InDaysF() const { return secs_ / SecondsPerDay; }
  constexpr double InHoursF() const { return secs_ / SecondsPerHour; }
  constexpr double InMinutesF() const { return secs_ / SecondsPerMinute; }
  constexpr double InSecondsF() const { return secs_; }
  constexpr double InMillisecondsF() const { return secs_ * MillisecondsPerSecond; }
  constexpr double InMicrosecondsF() const { return secs_ * MicrosecondsPerSecond; }

  // Computations with other deltas.
  TimeDeltaF operator+(TimeDeltaF other) const {
    return TimeDeltaF(secs_ + other.secs_);
  }
  TimeDeltaF operator-(TimeDeltaF other) const {
    return TimeDeltaF(secs_ - other.secs_);
  }

  TimeDeltaF& operator+=(TimeDeltaF other) {
    return *this = (*this + other);
  }
  TimeDeltaF& operator-=(TimeDeltaF other) {
    return *this = (*this - other);
  }

  TimeDeltaF operator-() const { return TimeDeltaF(-secs_); }

  // Computations with numeric types.
  template<typename T>
  TimeDeltaF operator*(T a) const {
    return TimeDeltaF(secs_ * a);
  }
  template<typename T>
  TimeDeltaF operator/(T a) const {
    return TimeDeltaF(secs_ / a);
  }
  template<typename T>
  TimeDeltaF operator%(T a) const {
    return TimeDeltaF(IeeeRemainder(secs_, a));
  }
  template<typename T>
  TimeDeltaF& operator*=(T a) {
    return *this = (*this * a);
  }
  template<typename T>
  TimeDeltaF& operator/=(T a) {
    return *this = (*this / a);
  }

  double operator/(TimeDeltaF a) const { return secs_ / a.secs_; }

  // Comparison operators.
  bool operator==(TimeDeltaF other) const { return secs_ == other.secs_; }
  bool operator!=(TimeDeltaF other) const { return secs_ != other.secs_; }
  bool operator< (TimeDeltaF other) const { return secs_ <  other.secs_; }
  bool operator<=(TimeDeltaF other) const { return secs_ <= other.secs_; }
  bool operator> (TimeDeltaF other) const { return secs_ >  other.secs_; }
  bool operator>=(TimeDeltaF other) const { return secs_ >= other.secs_; }

  void ToFormat(TextWriter& out, const StringSpan& opts) const;

  friend TimeDeltaF Abs(TimeDeltaF x) { return x.GetMagnitude(); }

 private:
  // Constructs a delta given the duration in microseconds. This is private
  // to avoid confusion by callers with a double constructor.
  // Use FromSecondsF, FromMillisecondsF, etc. instead.
  constexpr explicit TimeDeltaF(double delta_us) : secs_(delta_us) {}

  double secs_;
};

} // namespace stp

#endif // STP_BASE_TIME_TIMEDELTAF_H_
