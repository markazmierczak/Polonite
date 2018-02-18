// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CRYPTO_MD5_H_
#define STP_BASE_CRYPTO_MD5_H_

#include "Base/Containers/BufferSpan.h"

namespace stp {

class Md5Digest {
 public:
  static constexpr int Length = 16;

  enum NoInitTag { NoInit };
  explicit Md5Digest(NoInitTag) noexcept {}

  explicit Md5Digest(Span<byte_t> raw) noexcept {
    ASSERT(raw.size() == Length);
    UninitializedCopy(raw_, raw.data(), Length);
  }

  const byte_t& operator[](int pos) const noexcept {
    ASSERT(0 <= pos && pos < Length);
    return raw_[pos];
  }
  byte_t& operator[](int pos) noexcept {
    ASSERT(0 <= pos && pos < Length);
    return raw_[pos];
  }

  friend bool operator==(const Md5Digest& l, const Md5Digest& r) noexcept {
    return MakeSpan(l.raw_) == MakeSpan(r.raw_);
  }
  friend bool operator!=(const Md5Digest& l, const Md5Digest& r) noexcept {
    return !operator==(l, r);
  }

 private:
  byte_t raw_[Length];
};

BASE_EXPORT Md5Digest computeMd5Digest(BufferSpan input) noexcept;
BASE_EXPORT bool tryParse(StringSpan s, Md5Digest& out_digest) noexcept;

BASE_EXPORT void Format(TextWriter& out, const Md5Digest& digest, const StringSpan& opts);
BASE_EXPORT TextWriter& operator<<(TextWriter& out, const Md5Digest& digest);

class BASE_EXPORT Md5Hasher {
 public:
  Md5Hasher() noexcept { reset(); }

  void reset() noexcept;
  void update(BufferSpan input) noexcept;
  void finish(Md5Digest& out_digest) noexcept;

 private:
  void transform() noexcept;

  uint32_t buf_[4];
  uint32_t bits_[2];
  union {
    uint8_t in_[64];
    uint32_t inw_[16];
  };
};

} // namespace stp

#endif // STP_BASE_CRYPTO_MD5_H_
