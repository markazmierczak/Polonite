// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Call/Delegate.h"

#include "Base/Mem/OwnPtr.h"
#include "Base/Test/GTest.h"
#include "Base/Test/PerfTest.h"
#include "Base/Time/TimeTicks.h"

#include <functional>

namespace stp {

static const int BenchmarkIterations = 50000000;

struct CallbackHolder {
  virtual ~CallbackHolder() {}
  virtual void DoNothing(int, int, int) {}
};

struct DelegateHolder : public CallbackHolder {
  Delegate<void(int, int, int)> callback;
};

struct StdFunctionHolder : public CallbackHolder {
  std::function<void(int, int, int)> callback;
};

template<typename T>
static void RunDelegateBenchmark(T* callback_holder, const String& trace_name) {
  TimeTicks start = TimeTicks::Now();
  for (int i = 0; i < BenchmarkIterations; ++i) {
    callback_holder->callback(1, 2, i);
  }
  double total_time_milliseconds = (TimeTicks::Now() - start).InMillisecondsF();
  perf_test::PrintResult(
      "delegate_call", "", trace_name,
      BenchmarkIterations / total_time_milliseconds,
      "runs/ms", true);
}

TEST(DelegatePerfTest, Call) {
  auto delegate_holder = OwnPtr<DelegateHolder>::New();
  delegate_holder->callback = MakeDelegate(&DelegateHolder::DoNothing, delegate_holder.get());
  RunDelegateBenchmark(delegate_holder.get(), "delegate");

  auto std_function_holder = OwnPtr<StdFunctionHolder>::New();
  std_function_holder->callback = std::bind(
      &StdFunctionHolder::DoNothing, std_function_holder.get(),
      std::placeholders::_1,
      std::placeholders::_2,
      std::placeholders::_3);
  RunDelegateBenchmark(std_function_holder.get(), "std_function");
}

} // namespace stp
