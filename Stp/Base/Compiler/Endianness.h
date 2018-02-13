// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_COMPILER_ENDIANNESS_H_
#define STP_BASE_COMPILER_ENDIANNESS_H_

#include <stdint.h>

namespace stp {

enum class Endianness : uint8_t {
  Little,
  Big,

  Native = Little,
};

} // namespace stp

#endif // STP_BASE_COMPILER_ENDIANNESS_H_
