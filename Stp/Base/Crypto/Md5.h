// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CRYPTO_MD5_H_
#define STP_BASE_CRYPTO_MD5_H_

#include "Base/Containers/Span.h"

namespace stp {

class Md5Digest {
 public:
  static constexpr int Length = 16;

  enum NoInitTag { NoInit };
  explicit Md5Digest(NoInitTag) {}

  explicit Md5Digest(Span<byte_t> raw) {
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

  friend bool operator==(const Md5Digest& l, const Md5Digest& r) {
    return makeSpan(l.raw_) == makeSpan(r.raw_);
  }
  friend bool operator!=(const Md5Digest& l, const Md5Digest& r) {
    return !operator==(l, r);
  }

 private:
  byte_t raw_[Length];
};

BASE_EXPORT Md5Digest computeMd5Digest(BufferSpan input);
BASE_EXPORT bool tryParse(StringSpan s, Md5Digest& out_digest);

BASE_EXPORT void format(TextWriter& out, const Md5Digest& digest, const StringSpan& opts);
BASE_EXPORT TextWriter& operator<<(TextWriter& out, const Md5Digest& digest);

class Md5Hasher {
 public:
  Md5Hasher() { reset(); }

  BASE_EXPORT void reset();
  BASE_EXPORT void update(BufferSpan input);
  BASE_EXPORT void finish(Md5Digest& out_digest);

 private:
  void transform();

  uint32_t buf_[4];
  uint32_t bits_[2];
  union {
    uint8_t in_[64];
    uint32_t inw_[16];
  };
};

} // namespace stp

#endif // STP_BASE_CRYPTO_MD5_H_
