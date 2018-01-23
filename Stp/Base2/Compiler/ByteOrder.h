// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_COMPILER_BYTEORDER_H_
#define STP_BASE_COMPILER_BYTEORDER_H_

namespace stp {

enum class ByteOrder {
  LittleEndian,
  BigEndian,

  Native = LittleEndian,
};

} // namespace stp

#endif // STP_BASE_COMPILER_BYTEORDER_H_
