// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_BASE64_H_
#define STP_BASE_IO_BASE64_H_

#include "Base/Containers/Buffer.h"
#include "Base/Containers/List.h"

namespace stp {

class BASE_EXPORT Base64 {
  STATIC_ONLY(Base64);
 public:
  // Encodes the input bytes in base64.
  static String encode(BufferSpan input);

  // Decodes the base64 input string.
  [[nodiscard]] static bool tryDecode(StringSpan input, Buffer& output);

  // Low-level function - encodes input bytes and writes base64 representation
  // to the output. Returns number of characters written to the output.
  static int encode(MutableStringSpan output, BufferSpan input);

  static int estimateEncodedLength(int input_size);

  // Low-level function - decodes input characters in base64 and writes decoded bytes
  // to the output. Returns the number of characters decoded or negative number
  // if an error occurred.
  [[nodiscard]] static int tryDecode(StringSpan input, MutableBufferSpan output);

  static int estimateDecodedSize(int input_size);
};

inline int Base64::estimateEncodedLength(int input_size) {
  ASSERT(input_size >= 0);
  return (toUnsigned(input_size) + 2) / 3 * 4 + 1;
}

inline int Base64::estimateDecodedSize(int input_size) {
  ASSERT(input_size >= 0);
  return toUnsigned(input_size) / 4 * 3 + 2;
}

} // namespace stp

#endif // STP_BASE_IO_BASE64_H_
