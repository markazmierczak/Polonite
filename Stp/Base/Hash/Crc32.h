// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_HASH_CRC32_H_
#define STP_BASE_HASH_CRC32_H_

#include "Base/Containers/BufferSpan.h"
#include "Base/Type/FormattableFwd.h"

namespace stp {

enum class Crc32Value : uint32_t {};

BASE_EXPORT Crc32Value ComputeCrc32(BufferSpan input) noexcept;

BASE_EXPORT bool TryParse(StringSpan s, Crc32Value& out_checksum) noexcept;

BASE_EXPORT void Format(TextWriter& out, Crc32Value checksum, const StringSpan& opts);
BASE_EXPORT void Format(TextWriter& out, Crc32Value checksum);

inline TextWriter& operator<<(TextWriter& out, Crc32Value checksum) {
  Format(out, checksum); return out;
}

class BASE_EXPORT Crc32Algorithm {
 public:
  Crc32Algorithm() = default;

  void Reset() { residue_ = InitialResidue; }
  void Update(BufferSpan input) noexcept;
  Crc32Value GetChecksum() const { return static_cast<Crc32Value>(~residue_); }

 private:
  static const uint32_t InitialResidue = 0xFFFFFFFF;

  uint32_t residue_ = InitialResidue;
};

} // namespace stp

#endif // STP_BASE_HASH_CRC32_H_
