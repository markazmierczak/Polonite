// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_FINALLY_H_
#define STP_BASE_UTIL_FINALLY_H_

#include "Base/Error/ExceptionFwd.h"
#include "Base/Type/Limits.h"
#include "Base/Type/Variable.h"

namespace stp {

namespace detail {

struct ScopeFinallyCondition {
  static constexpr bool MayThrow = false;

  bool cancelled = false;

  void Cancel() { cancelled = true; }
  bool ShouldExecute() const { return !cancelled; }
};

struct ScopeCatchCondition {
  static constexpr bool MayThrow = false;

  int exception_count = CountUncaughtExceptions();

  void Cancel() { exception_count = Limits<int>::Max; }
  bool ShouldExecute() const { return exception_count < CountUncaughtExceptions(); }
};

struct ScopeContinueCondition {
  static constexpr bool MayThrow = true;

  int exception_count = CountUncaughtExceptions();

  void Cancel() { exception_count = -1; }
  bool ShouldExecute() const { return exception_count >= CountUncaughtExceptions(); }
};

template<typename TAction, typename TCondition>
class ScopeGuard {
 private:
  TAction action_;
  TCondition condition_;

 public:
  typedef TCondition ConditionType;

  explicit ScopeGuard(TAction action) noexcept(noexcept(TAction(Move(action))))
      : action_(Move(action)) {}

  ~ScopeGuard() noexcept(!TCondition::MayThrow || noexcept(action_())) {
    if (condition_.ShouldExecute())
      action_();
  }

  ScopeGuard(ScopeGuard&& other)
      : action_(Move(other.action_)),
        condition_(other.condition_) {
    other.Cancel();
  }

  void Cancel() { condition_.Cancel(); }

  template<typename T>
  T CancelWithResult(T x) { Cancel(); return x; }

  DISALLOW_COPY_AND_ASSIGN(ScopeGuard);
};

} // namespace detail

template<typename TAction>
using ScopeFinally = detail::ScopeGuard<TAction, detail::ScopeFinallyCondition>;
template<typename TAction>
using ScopeCatch = detail::ScopeGuard<TAction, detail::ScopeCatchCondition>;
template<typename TAction>
using ScopeContinue = detail::ScopeGuard<TAction, detail::ScopeContinueCondition>;

template<typename TAction, typename TDecayed = TDecay<TAction>>
inline auto MakeScopeFinally(TAction&& f) noexcept(noexcept(TDecayed(Forward<TAction>(f)))) {
  return ScopeFinally<TDecayed>(Forward<TAction>(f));
}

template<typename TAction, typename TDecayed = TDecay<TAction>>
inline auto MakeScopeCatch(TAction&& f) noexcept(noexcept(TDecayed(Forward<TAction>(f)))) {
  return ScopeCatch<TDecayed>(Forward<TAction>(f));
}

template<typename TAction, typename TDecayed = TDecay<TAction>>
inline auto MakeScopeContinue(TAction&& f) noexcept(noexcept(TDecayed(Forward<TAction>(f)))) {
  return ScopeContinue<TDecayed>(Forward<TAction>(f));
}

} // namespace stp

#endif // STP_BASE_UTIL_FINALLY_H_
