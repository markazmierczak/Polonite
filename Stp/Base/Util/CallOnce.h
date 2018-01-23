// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_CALLONCE_H_
#define STP_BASE_UTIL_CALLONCE_H_

#include "Base/Sync/AtomicOps.h"
#include "Base/Type/CVRef.h"

namespace stp {

#define CALL_ONCE_INITIALIZER { stp::CallOnce::NotStarted }

class BASE_EXPORT CallOnce {
 public:
  template<typename TFunction, typename... TArgs>
  void operator()(TFunction&& fn, TArgs&&... args) {
    auto state = subtle::Acquire_Load(&state_);
    if (state == Done)
      return;

    if (state == NotStarted && NeedsCall(&state_)) {
      fn(Forward<TArgs>(args)...);

      subtle::Release_Store(&state_, Done);
    }
  }

  enum State : subtle::Atomic32 {
    NotStarted,
    Claimed,
    Done,
  };

  subtle::Atomic32 state_;

  static bool NeedsCall(subtle::Atomic32* state);
};

} // namespace stp

#endif // STP_BASE_UTIL_CALLONCE_H_
