// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CRYPTO_CRC32_H_
#define STP_BASE_CRYPTO_CRC32_H_

#include "Base/Containers/BufferSpan.h"

namespace stp {

enum class Crc32Value : uint32_t {};

BASE_EXPORT Crc32Value computeCrc32(BufferSpan input);

BASE_EXPORT bool tryParse(StringSpan s, Crc32Value& out_checksum);

BASE_EXPORT void format(TextWriter& out, Crc32Value checksum, const StringSpan& opts);

BASE_EXPORT TextWriter& operator<<(TextWriter& out, Crc32Value checksum);

class Crc32Algorithm {
 public:
  Crc32Algorithm() = default;

  void reset() { residue_ = InitialResidue; }
  BASE_EXPORT void update(BufferSpan input);
  Crc32Value getChecksum() const { return static_cast<Crc32Value>(~residue_); }

 private:
  static constexpr uint32_t InitialResidue = 0xFFFFFFFF;

  uint32_t residue_ = InitialResidue;
};

} // namespace stp

#endif // STP_BASE_CRYPTO_CRC32_H_
