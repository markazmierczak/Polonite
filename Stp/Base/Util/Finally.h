// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_FINALLY_H_
#define STP_BASE_UTIL_FINALLY_H_

#include "Base/Error/Exception.h"
#include "Base/Type/Limits.h"

namespace stp {

namespace detail {

struct ScopeFinallyCondition {
  static constexpr bool MayThrow = false;

  bool cancelled = false;

  void cancel() { cancelled = true; }
  bool shouldExecute() const { return !cancelled; }
};

struct ScopeCatchCondition {
  static constexpr bool MayThrow = false;

  int exception_count = countUncaughtExceptions();

  void cancel() { exception_count = Limits<int>::Max; }
  bool shouldExecute() const { return exception_count < countUncaughtExceptions(); }
};

struct ScopeContinueCondition {
  static constexpr bool MayThrow = true;

  int exception_count = countUncaughtExceptions();

  void cancel() { exception_count = -1; }
  bool shouldExecute() const { return exception_count >= countUncaughtExceptions(); }
};

template<typename TAction, typename TCondition>
class ScopeGuard {
 private:
  TAction action_;
  TCondition condition_;

 public:
  typedef TCondition ConditionType;

  explicit ScopeGuard(TAction action) noexcept(noexcept(TAction(move(action))))
      : action_(move(action)) {}

  ~ScopeGuard() noexcept(!TCondition::MayThrow || noexcept(action_())) {
    if (condition_.shouldExecute())
      action_();
  }

  ScopeGuard(ScopeGuard&& other)
      : action_(move(other.action_)),
        condition_(other.condition_) {
    other.cancel();
  }

  void cancel() { condition_.cancel(); }

  template<typename T>
  T cancelWithResult(T x) { cancel(); return x; }

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
inline auto makeScopeFinally(TAction&& f) noexcept(noexcept(TDecayed(forward<TAction>(f)))) {
  return ScopeFinally<TDecayed>(forward<TAction>(f));
}

template<typename TAction, typename TDecayed = TDecay<TAction>>
inline auto makeScopeCatch(TAction&& f) noexcept(noexcept(TDecayed(forward<TAction>(f)))) {
  return ScopeCatch<TDecayed>(forward<TAction>(f));
}

template<typename TAction, typename TDecayed = TDecay<TAction>>
inline auto makeScopeContinue(TAction&& f) noexcept(noexcept(TDecayed(forward<TAction>(f)))) {
  return ScopeContinue<TDecayed>(forward<TAction>(f));
}

} // namespace stp

#endif // STP_BASE_UTIL_FINALLY_H_
