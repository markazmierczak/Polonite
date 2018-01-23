// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TIME_TIMELITERALS_H_
#define STP_BASE_TIME_TIMELITERALS_H_

#include "Base/Time/TimeDelta.h"
#include "Base/Time/TimeDeltaF.h"

namespace stp {

inline namespace time_literals {

constexpr TimeDelta operator"" _h(int s) {
  return TimeDelta::FromHours(s);
}

constexpr TimeDelta operator"" _min(int s) {
  return TimeDelta::FromMinutes(s);
}

constexpr TimeDelta operator"" _s(int64_t s) {
  return TimeDelta::FromSeconds(s);
}

constexpr TimeDelta operator"" _ms(int64_t s) {
  return TimeDelta::FromMilliseconds(s);
}

constexpr TimeDelta operator"" _us(int64_t s) {
  return TimeDelta::FromMicroseconds(s);
}

constexpr TimeDeltaF operator"" _h(double s) {
  return TimeDeltaF::FromHoursF(s);
}

constexpr TimeDeltaF operator"" _min(double s) {
  return TimeDeltaF::FromMinutesF(s);
}

constexpr TimeDelta operator"" _s(double s) {
  return TimeDeltaF::FromSecondsF(s);
}

constexpr TimeDeltaF operator"" _ms(double s) {
  return TimeDeltaF::FromMillisecondsF(s);
}

constexpr TimeDeltaF operator"" _us(double s) {
  return TimeDeltaF::FromMicrosecondsF(s);
}

} // namespace time_literals

} // namespace stp

#endif // STP_BASE_TIME_TIMELITERALS_H_
