// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_TIME_TIMEBASE_H_
#define STP_BASE_TIME_TIMEBASE_H_

#include "Base/Time/TimeDelta.h"
#include "Base/Type/Limits.h"

namespace stp {

// Do not reference the TimeBase template class directly.  Please
// use one of the time subclasses instead, and only reference the public
// TimeBase members via those classes.
//
// Provides value storage and comparison/math operations common to all time
// classes. Each subclass provides for strong type-checking to ensure
// semantically meaningful comparison/math of time values from the same clock
// source or timeline.
template<class TimeClass>
class TimeBase {
 public:
  // Returns true if this object has not been initialized.
  //
  // Warning: Be careful when writing code that performs math on time values,
  // since it's possible to produce a valid "zero" result that should not be
  // interpreted as a "null" value.
  bool isNull() const { return us_ == 0; }

  // Made public for serialization only.
  ALWAYS_INLINE int64_t ToInternalValue() const { return us_; }
  static ALWAYS_INLINE TimeClass FromInternalValue(int64_t us) { return TimeClass(us); }

  TimeClass& operator=(TimeClass other) {
    us_ = other.us_;
    return *(static_cast<TimeClass*>(this));
  }

  // Compute the difference between two times.
  TimeDelta operator-(TimeClass other) const {
    return TimeDelta::FromMicroseconds(us_ - other.us_);
  }

  // Return a new time modified by some delta.
  TimeClass operator+(TimeDelta delta) const {
    return FromInternalValue(MakeSafe(us_) + delta.ToInternalValue());
  }
  TimeClass operator-(TimeDelta delta) const {
    return FromInternalValue(MakeSafe(us_) - delta.ToInternalValue());
  }

  // Modify by some time delta.
  TimeClass& operator+=(TimeDelta delta) {
    return static_cast<TimeClass&>(*this = (*this + delta));
  }
  TimeClass& operator-=(TimeDelta delta) {
    return static_cast<TimeClass&>(*this = (*this - delta));
  }

  // Comparison operators
  bool operator==(TimeClass other) const { return us_ == other.us_; }
  bool operator!=(TimeClass other) const { return us_ != other.us_; }
  bool operator< (TimeClass other) const { return us_ <  other.us_; }
  bool operator<=(TimeClass other) const { return us_ <= other.us_; }
  bool operator> (TimeClass other) const { return us_ >  other.us_; }
  bool operator>=(TimeClass other) const { return us_ >= other.us_; }

 protected:
  explicit TimeBase(int64_t us) : us_(us) {}

  // Time value in a microsecond timebase.
  int64_t us_;
};

template<class TimeClass>
inline TimeClass operator+(TimeDelta delta, TimeClass t) {
  return t + delta;
}

} // namespace stp

#endif // STP_BASE_TIME_TIMEBASE_H_
