// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CRYPTO_SHA1_H_
#define STP_BASE_CRYPTO_SHA1_H_

#include "Base/Containers/Span.h"

namespace stp {

class Sha1Digest {
 public:
  static constexpr int Length = 20;

  enum NoInitTag { NoInit };
  explicit Sha1Digest(NoInitTag) {}

  explicit Sha1Digest(Span<byte_t> raw) {
    ASSERT(raw.size() == Length);
    uninitializedCopy(raw_, raw.data(), Length);
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
    return makeSpan(l.raw_) == makeSpan(r.raw_);
  }
  friend bool operator!=(const Sha1Digest& l, const Sha1Digest& r) {
    return !operator==(l, r);
  }

 private:
  byte_t raw_[Length];
};

BASE_EXPORT Sha1Digest computeSha1Digest(BufferSpan input);
BASE_EXPORT bool tryParse(StringSpan s, Sha1Digest& out_digest);

BASE_EXPORT void format(TextWriter& out, const Sha1Digest& digest, const StringSpan& opts);
BASE_EXPORT TextWriter& operator<<(TextWriter& out, const Sha1Digest& digest);

class Sha1Hasher {
 public:
  Sha1Hasher() { reset(); }

  BASE_EXPORT void reset();
  BASE_EXPORT void update(BufferSpan input);
  BASE_EXPORT void finish(Sha1Digest& out_digest);

 private:
  void pad();
  void process();

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
