// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_BUILD_H_
#define STP_BASE_UTIL_BUILD_H_

#include "Base/Type/Attributes.h"

namespace stp {

class Time;

class Build {
  STATIC_ONLY(Build);
 public:
  #if defined(NDEBUG)
  static constexpr bool IsDebug = false;
  #else
  static constexpr bool IsDebug = true;
  #endif

  static Time getTranslationTime();
};

} // namespace stp

#endif // STP_BASE_UTIL_BUILD_H_
