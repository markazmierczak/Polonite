// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MATH_FLUSHTOZERO_H_
#define STP_BASE_MATH_FLUSHTOZERO_H_

#include "Base/Compiler/Simd.h"
#include "Base/Type/Attributes.h"

namespace stp {

// Turns on "denormals are zero" and "flush to zero" CSR flags.
//
// Both of these modes are less precise than using the IEEE 754 standard
// method,  and so they are not compatible with the standard, but do
// provide substantially  faster execution times when absolute accuracy
// according to the standard is not needed.
class ScopedSubnormalFloatDisabler {
 public:
  ScopedSubnormalFloatDisabler() {
    #if CPU_SIMD(SSE1)
    orig_state_ = _mm_getcsr();
    _mm_setcsr(orig_state_ | 0x8040);
    #endif
  }

  ~ScopedSubnormalFloatDisabler() {
    #if CPU_SIMD(SSE1)
    _mm_setcsr(orig_state_);
    #endif
  }

 private:
  #if CPU_SIMD(SSE1)
  unsigned int orig_state_;
  #endif
  DISALLOW_COPY_AND_ASSIGN(ScopedSubnormalFloatDisabler);
};

} // namespace stp

#endif // STP_BASE_MATH_FLUSHTOZERO_H_
