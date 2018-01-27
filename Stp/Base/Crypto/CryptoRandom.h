// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_CRYPTO_CRYPTORANDOM_H_
#define STP_BASE_CRYPTO_CRYPTORANDOM_H_

#include "Base/Containers/BufferSpan.h"

namespace stp {

class BASE_EXPORT CryptoRandom {
 public:
  CryptoRandom() = default;

  void Fill(MutableBufferSpan buffer) noexcept;

  uint32_t NextUInt32() noexcept { return GenerateTrivial<uint32_t>(); }
  uint64_t NextUInt64() noexcept { return GenerateTrivial<uint64_t>(); }

 private:
  template<typename T>
  T GenerateTrivial() noexcept {
    T x;
    Fill(MutableBufferSpan(&x, 1));
    return x;
  }
};

} // namespace stp

#endif // STP_BASE_CRYPTO_CRYPTORANDOM_H_
