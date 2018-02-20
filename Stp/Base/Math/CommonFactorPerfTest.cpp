// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Math/CommonFactor.h"

#include "Base/Containers/List.h"
#include "Base/Crypto/CryptoRandom.h"
#include "Base/Test/GTest.h"
#include "Base/Test/PerfTest.h"
#include "Base/Time/TimeTicks.h"

namespace stp {

static const int GcdBenchmarkIterations = 1000000;

template<typename T>
static List<T> generateTable() {
  int size = GcdBenchmarkIterations + 1;
  List<T> list;
  T* dst = list.appendUninitialized(size);
  CryptoRandom rng;
  for (int i = 0; i < size; ++i)
    dst[i] = static_cast<T>(rng.nextUint64());
  return list;
}

template<typename T>
static inline T simpleGcd(T a, T b) {
  T tmp = 0;
  while (b) {
    tmp = a;
    a = b;
    b = tmp % b;
  }
  return a;
}

TEST(GcdPerfTest, euclid32) {
  auto table = generateTable<uint32_t>();

  TimeTicks start = TimeTicks::Now();
  for (int i = 0; i < GcdBenchmarkIterations; ++i) {
    simpleGcd(table[i], table[i + 1]);
  }
  double total_time_milliseconds = (TimeTicks::Now() - start).InMillisecondsF();
  perf_test::PrintResult(
      "gcd", "euclid", "32",
      GcdBenchmarkIterations / total_time_milliseconds,
      "runs/ms", true);
}

TEST(GcdPerfTest, binary32) {
  auto table = generateTable<uint32_t>();

  TimeTicks start = TimeTicks::Now();
  for (int i = 0; i < GcdBenchmarkIterations; ++i) {
    greatestCommonDivisor(table[i], table[i + 1]);
  }
  double total_time_milliseconds = (TimeTicks::Now() - start).InMillisecondsF();
  perf_test::PrintResult(
      "gcd", "binary", "32",
      GcdBenchmarkIterations / total_time_milliseconds,
      "runs/ms", true);
}

TEST(GcdPerfTest, euclid64) {
  auto table = generateTable<uint64_t>();

  TimeTicks start = TimeTicks::Now();
  for (int i = 0; i < GcdBenchmarkIterations; ++i) {
    simpleGcd(table[i], table[i + 1]);
  }
  double total_time_milliseconds = (TimeTicks::Now() - start).InMillisecondsF();
  perf_test::PrintResult(
      "gcd", "euclid", "64",
      GcdBenchmarkIterations / total_time_milliseconds,
      "runs/ms", true);
}

TEST(GcdPerfTest, binary64) {
  auto table = generateTable<uint64_t>();

  TimeTicks start = TimeTicks::Now();
  for (int i = 0; i < GcdBenchmarkIterations; ++i) {
    greatestCommonDivisor(table[i], table[i + 1]);
  }
  double total_time_milliseconds = (TimeTicks::Now() - start).InMillisecondsF();
  perf_test::PrintResult(
      "gcd", "binary", "64",
      GcdBenchmarkIterations / total_time_milliseconds,
      "runs/ms", true);
}

} // namespace stp
