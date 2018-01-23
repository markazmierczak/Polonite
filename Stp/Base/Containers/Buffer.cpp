// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/Buffer.h"

namespace stp {

bool TryParse(StringSpan input, Buffer& output) {
  if (input.size() & 1)
    return false;

  output.Clear();

  int output_size = input.size() >> 1;
  byte_t* dst = output.AppendUninitialized(output_size);
  return TryParse(input, MutableBufferSpan(dst, output_size));
}

} // namespace stp
