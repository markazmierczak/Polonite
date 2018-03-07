// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_FINALLY_H_
#define STP_BASE_UTIL_FINALLY_H_

#include "Base/Type/Limits.h"
#include "Base/Type/Variable.h"

namespace stp {

template<typename TAction>
class ScopeFinally {
  DISALLOW_COPY_AND_ASSIGN(ScopeFinally);
 public:
  explicit ScopeFinally(TAction action) : action_(stp::move(action)) {}

  ~ScopeFinally() {
    if (!cancelled_)
      action_();
  }

  ScopeFinally(ScopeFinally&& other)
      : action_(stp::move(other.action_)),
        cancelled_(stp::exchange(other.cancelled_, true)) {
    other.cancel();
  }

  void cancel() { cancelled_ = true; }

  template<typename T>
  T cancelWithResult(T x) { cancel(); return x; }

 private:
  TAction action_;

  bool cancelled_ = false;
};

template<typename TAction, typename TDecayed = TDecay<TAction>>
inline auto makeScopeFinally(TAction&& f) {
  return ScopeFinally<TDecayed>(forward<TAction>(f));
}

} // namespace stp

#endif // STP_BASE_UTIL_FINALLY_H_
