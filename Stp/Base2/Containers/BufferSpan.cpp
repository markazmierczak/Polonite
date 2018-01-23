// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/BufferSpan.h"

#include "Base/Text/AsciiChar.h"

namespace stp {

bool TryParse(StringSpan input, MutableBufferSpan output) {
  if (input.size() & 1)
    return false;

  int num_to_output = input.size() >> 1;
  if (num_to_output != output.size())
    return false;

  auto* output_bytes = static_cast<byte_t*>(output.data());
  for (int i = 0; i < num_to_output; ++i) {
    char mc = input[i * 2 + 0];
    char lc = input[i * 2 + 1];

    int msb = TryParseHexDigit(mc);
    int lsb = TryParseHexDigit(lc);
    if (msb < 0 || lsb < 0)
      return false;

    output_bytes[i] = static_cast<byte_t>((msb << 4) | lsb);
  }
  return true;
}

} // namespace stp
