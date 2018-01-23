// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_HASH_CRC32_H_
#define STP_BASE_HASH_CRC32_H_

#include "Base/Containers/BufferSpan.h"
#include "Base/Type/FormattableFwd.h"

namespace stp {

enum class Crc32Digest : uint32_t {};

BASE_EXPORT Crc32Digest ComputeCrc32Digest(BufferSpan input);

BASE_EXPORT bool TryParse(StringSpan s, Crc32Digest& out_digest);

BASE_EXPORT void Format(TextWriter& out, Crc32Digest digest, const StringSpan& opts);
BASE_EXPORT void Format(TextWriter& out, Crc32Digest digest);

inline TextWriter& operator<<(TextWriter& out, Crc32Digest digest) {
  Format(out, digest); return out;
}

class BASE_EXPORT Crc32Hasher {
 public:
  typedef Crc32Digest Digest;

  Crc32Hasher() { Reset(); }

  void Reset() { context_ = 0; }
  void Update(BufferSpan input);
  void Finish(Digest& output) { output = static_cast<Crc32Digest>(context_); }

 private:
  uint32_t context_;

  static const uint32_t Table_[256];
};

} // namespace stp

#endif // STP_BASE_HASH_CRC32_H_
