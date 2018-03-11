// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CRYPTO_CRYPTORANDOM_H_
#define STP_BASE_CRYPTO_CRYPTORANDOM_H_

#include "Base/Containers/BufferSpan.h"

namespace stp {

class BASE_EXPORT CryptoRandom {
 public:
  CryptoRandom() = default;

  void generate(MutableBufferSpan buffer);

  uint32_t nextUint32() { return generateTrivial<uint32_t>(); }
  uint64_t nextUint64() { return generateTrivial<uint64_t>(); }

 private:
  template<typename T>
  T generateTrivial() {
    T x;
    generate(MutableBufferSpan(&x, 1));
    return x;
  }
};

} // namespace stp

#endif // STP_BASE_CRYPTO_CRYPTORANDOM_H_
