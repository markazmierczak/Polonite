// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CRYPTO_SHA1_H_
#define STP_BASE_CRYPTO_SHA1_H_

#include "Base/Containers/BufferSpan.h"

namespace stp {

class Sha1Digest {
 public:
  static constexpr int Length = 20;

  explicit Sha1Digest(NoInitTag) {}

  explicit Sha1Digest(Span<byte_t> raw) {
    ASSERT(raw.size() == Length);
    UninitializedCopy(raw_, raw.data(), Length);
  }

  const byte_t& operator[](int pos) const {
    ASSERT(0 <= pos && pos < Length);
    return raw_[pos];
  }
  byte_t& operator[](int pos) {
    ASSERT(0 <= pos && pos < Length);
    return raw_[pos];
  }

  friend bool operator==(const Sha1Digest& l, const Sha1Digest& r) {
    return MakeSpan(l.raw_) == MakeSpan(r.raw_);
  }
  friend bool operator!=(const Sha1Digest& l, const Sha1Digest& r) {
    return !operator==(l, r);
  }

 private:
  byte_t raw_[Length];
};

BASE_EXPORT Sha1Digest ComputeSha1Digest(BufferSpan input) noexcept;
BASE_EXPORT bool TryParse(StringSpan s, Sha1Digest& out_digest) noexcept;

BASE_EXPORT void Format(TextWriter& out, const Sha1Digest& digest, const StringSpan& opts);
BASE_EXPORT void Format(TextWriter& out, const Sha1Digest& digest);

inline TextWriter& operator<<(TextWriter& out, const Sha1Digest& digest) {
  Format(out, digest); return out;
}

class BASE_EXPORT Sha1Hasher {
 public:
  Sha1Hasher() noexcept { Reset(); }

  void Reset() noexcept;
  void Update(BufferSpan input) noexcept;
  void Finish(Sha1Digest& out_digest) noexcept;

 private:
  void Pad() noexcept;
  void Process() noexcept;

  uint32_t a_, b_, c_, d_, e_;

  uint32_t h_[5];

  union {
    uint32_t w_[80];
    uint8_t m_[64];
  };

  uint32_t cursor_;
  uint64_t l_;
};

} // namespace stp

#endif // STP_BASE_CRYPTO_SHA1_H_
