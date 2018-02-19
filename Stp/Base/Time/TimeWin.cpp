// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Time/Time.h"

#include "Base/System/CpuInfo.h"
#include "Base/Thread/Lock.h"
#include "Base/Thread/NativeThread.h"
#include "Base/Time/ThreadTicks.h"
#include "Base/Time/TimeTicks.h"
#include "Base/Type/Variable.h"

#include <mmsystem.h>
#include <windows.h>

namespace stp {

namespace {

// From MSDN, FILETIME "contains a 64-bit value representing the number of
// 100-nanosecond intervals since January 1, 1601 (UTC)."
int64_t FileTimeToMicroseconds(const FILETIME& ft) {
  // Need to bit_cast to fix alignment, then divide by 10 to convert
  // 100-nanoseconds to microseconds. This only works on little-endian machines.
  return bit_cast<int64_t, FILETIME>(ft) / 10;
}

FILETIME MicrosecondsToFileTime(int64_t us) {
  ASSERT(us >= 0, "time is less than 0, negative values are not representable in FILETIME");

  // Multiply by 10 to convert microseconds to 100-nanoseconds. Bit_cast will
  // handle alignment problems. This only works on little-endian machines.
  return bit_cast<FILETIME>(us * 10);
}

int64_t CurrentWallclockMicroseconds() {
  FILETIME ft;
  ::GetSystemTimeAsFileTime(&ft);
  return FileTimeToMicroseconds(ft);
}

// How many times the high resolution timer has been called.
uint32_t g_high_res_timer_count = 0;
// The lock to control access to the above two variables.
BasicLock g_high_res_lock = BASIC_LOCK_INITIALIZER;

// Returns the current value of the performance counter.
inline uint64_t QPCNowRaw() {
  LARGE_INTEGER perf_counter_now;
  // According to the MSDN documentation for QueryPerformanceCounter(), this
  // will never fail on systems that run XP or later.
  // https://msdn.microsoft.com/library/windows/desktop/ms644904.aspx
  ::QueryPerformanceCounter(&perf_counter_now);
  return perf_counter_now.QuadPart;
}

} // namespace

// Time -----------------------------------------------------------------------

Time Time::Now() {
  return Time(CurrentWallclockMicroseconds() - WindowsEpochDeltaMicroseconds);
}

Time Time::FromFileTime(FILETIME ft) {
  if (bit_cast<int64_t, FILETIME>(ft) == 0)
    return Time();

  return Time(FileTimeToMicroseconds(ft) - WindowsEpochDeltaMicroseconds);
}

FILETIME Time::ToFileTime() const {
  if (isNull())
    return bit_cast<FILETIME, int64_t>(0);

  return MicrosecondsToFileTime(us_ + WindowsEpochDeltaMicroseconds);
}

void Time::ActivateHighResolutionTimer(bool activating) {
  const int kMinTimerIntervalHighResMs = 1;

  AutoLock lock(&g_high_res_lock);
  UINT period = kMinTimerIntervalHighResMs;
  if (activating) {
    ASSERT(g_high_res_timer_count != UINT32_MAX);
    ++g_high_res_timer_count;
    if (g_high_res_timer_count == 1)
      timeBeginPeriod(period);
  } else {
    ASSERT(g_high_res_timer_count != 0);
    --g_high_res_timer_count;
    if (g_high_res_timer_count == 0)
      timeEndPeriod(period);
  }
}

bool Time::IsHighResolutionTimerInUse() {
  AutoLock lock(&g_high_res_lock);
  return g_high_res_timer_count > 0;
}

bool Time::FromExploded(bool is_local, const Exploded& exploded, Time* time) {
  // Create the system struct representing our exploded time. It will either be
  // in local time or UTC.
  SYSTEMTIME st;
  st.wYear = static_cast<WORD>(exploded.year);
  st.wMonth = static_cast<WORD>(exploded.month);
  st.wDayOfWeek = static_cast<WORD>(exploded.day_of_week);
  st.wDay = static_cast<WORD>(exploded.day_of_month);
  st.wHour = static_cast<WORD>(exploded.hour);
  st.wMinute = static_cast<WORD>(exploded.minute);
  st.wSecond = static_cast<WORD>(exploded.second);
  st.wMilliseconds = static_cast<WORD>(exploded.millisecond);

  FILETIME ft;
  bool success = true;
  // Ensure that it's in UTC.
  if (is_local) {
    SYSTEMTIME utc_st;
    success = TzSpecificLocalTimeToSystemTime(nullptr, &st, &utc_st) &&
              SystemTimeToFileTime(&utc_st, &ft);
  } else {
    success = !!SystemTimeToFileTime(&st, &ft);
  }

  if (!success)
    return false;

  *time = Time(FileTimeToMicroseconds(ft) - WindowsEpochDeltaMicroseconds);
  return true;
}

void Time::Explode(bool is_local, Exploded* exploded) const {
  int64_t us_offseted = us_ + WindowsEpochDeltaMicroseconds;
  if (us_offseted < 0) {
    // We are not able to convert it to FILETIME.
    ZeroMemory(exploded, sizeof(*exploded));
    return;
  }

  // FILETIME in UTC.
  FILETIME utc_ft = MicrosecondsToFileTime(us_offseted);

  // FILETIME in local time if necessary.
  bool success = true;
  // FILETIME in SYSTEMTIME (exploded).
  SYSTEMTIME st = {0};
  if (is_local) {
    SYSTEMTIME utc_st;
    // We don't use FileTimeToLocalFileTime here, since it uses the current
    // settings for the time zone and daylight saving time. Therefore, if it is
    // daylight saving time, it will take daylight saving time into account,
    // even if the time you are converting is in standard time.
    success = FileTimeToSystemTime(&utc_ft, &utc_st) &&
        SystemTimeToTzSpecificLocalTime(nullptr, &utc_st, &st);
  } else {
    success = !!FileTimeToSystemTime(&utc_ft, &st);
  }

  if (!success) {
    ASSERT(false, "unable to convert time, don't know why");
    ZeroMemory(exploded, sizeof(*exploded));
    return;
  }

  exploded->year = st.wYear;
  exploded->month = st.wMonth;
  exploded->day_of_week = st.wDayOfWeek;
  exploded->day_of_month = st.wDay;
  exploded->hour = st.wHour;
  exploded->minute = st.wMinute;
  exploded->second = st.wSecond;
  exploded->millisecond = st.wMilliseconds;
}

// TimeTicks ------------------------------------------------------------------

static int64_t QPCTicksPerSecond_ = 0;

void TimeTicks::ClassInit() {
  LARGE_INTEGER ticks_per_sec;
  BOOL qpf_ok = QueryPerformanceFrequency(&ticks_per_sec);

  ASSERT_UNUSED(qpf_ok, qpf_ok);
  ASSERT(ticks_per_sec.QuadPart > 0);

  QPCTicksPerSecond_ = ticks_per_sec.QuadPart;
}

TimeTicks TimeTicks::FromQPCValue(LONGLONG qpc_value) {
  // To avoid overflow in QPC to Microseconds calculations, since we multiply
  // by MicrosecondsPerSecond, then the QPC value should not exceed
  // (2^63 - 1) / 1E6. If it exceeds that threshold, we divide then multiply.
  constexpr int64_t QPCOverflowThreshold = INT64_C(0x8637BD05AF7);

  ASSERT(QPCTicksPerSecond_ > 0);

  // If the QPC Value is below the overflow threshold, we proceed with
  // simple multiply and divide.
  if (qpc_value < QPCOverflowThreshold) {
    int64_t us = qpc_value * TimeDelta::MicrosecondsPerSecond / QPCTicksPerSecond_;
    return TimeTicks::FromInternalValue(us);
  }

  // Otherwise, calculate microseconds in a round about manner to avoid
  // overflow and precision issues.
  int64_t whole_seconds = qpc_value / QPCTicksPerSecond_;
  int64_t leftover_ticks = qpc_value - (whole_seconds * QPCTicksPerSecond_);

  int64_t us = whole_seconds * TimeDelta::MicrosecondsPerSecond;
  us += (leftover_ticks * TimeDelta::MicrosecondsPerSecond) / QPCTicksPerSecond_;
  return TimeTicks::FromInternalValue(us);
}

TimeTicks TimeTicks::Now() {
  return TimeTicks::FromQPCValue(QPCNowRaw());
}

ThreadTicks ThreadTicks::Now() {
  return ThreadTicks::GetForThread(NativeThread::currentHandle());
}

ThreadTicks ThreadTicks::GetForThread(HANDLE thread_handle) {
  ASSERT(IsSupported());

  // Get the number of TSC ticks used by the current thread.
  ULONG64 thread_cycle_time = 0;
  ::QueryThreadCycleTime(thread_handle, &thread_cycle_time);

  // Get the frequency of the TSC.
  double tsc_ticks_per_second = TSCTicksPerSecond();
  if (tsc_ticks_per_second == 0)
    return ThreadTicks();

  // Return the CPU time of the current thread.
  double thread_time_seconds = thread_cycle_time / tsc_ticks_per_second;
  return ThreadTicks(
      static_cast<int64_t>(thread_time_seconds * TimeDelta::MicrosecondsPerSecond));
}

bool ThreadTicks::IsSupportedWin() {
  static bool g_is_supported = CpuInfo::Supports(CpuFeature::NonStopTsc);
  return g_is_supported;
}

void ThreadTicks::WaitUntilInitializedWin() {
  while (TSCTicksPerSecond() == 0)
    ::Sleep(10);
}

double ThreadTicks::TSCTicksPerSecond() {
  ASSERT(IsSupported());

  // The value returned by QueryPerformanceFrequency() cannot be used as the TSC
  // frequency, because there is no guarantee that the TSC frequency is equal to
  // the performance counter frequency.

  // The TSC frequency is cached in a static variable because it takes some time
  // to compute it.
  static double tsc_ticks_per_second = 0;
  if (tsc_ticks_per_second != 0)
    return tsc_ticks_per_second;

  // Increase the thread priority to reduces the chances of having a context
  // switch during a reading of the TSC and the performance counter.
  int previous_priority = ::GetThreadPriority(::GetCurrentThread());
  ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

  // The first time that this function is called, make an initial reading of the
  // TSC and the performance counter.
  static const uint64_t tsc_initial = __rdtsc();
  static const uint64_t perf_counter_initial = QPCNowRaw();

  // Make a another reading of the TSC and the performance counter every time
  // that this function is called.
  uint64_t tsc_now = __rdtsc();
  uint64_t perf_counter_now = QPCNowRaw();

  // Reset the thread priority.
  ::SetThreadPriority(::GetCurrentThread(), previous_priority);

  // Make sure that at least 50 ms elapsed between the 2 readings. The first
  // time that this function is called, we don't expect this to be the case.
  // Note: The longer the elapsed time between the 2 readings is, the more
  //   accurate the computed TSC frequency will be. The 50 ms value was
  //   chosen because local benchmarks show that it allows us to get a
  //   stddev of less than 1 tick/us between multiple runs.
  // Note: According to the MSDN documentation for QueryPerformanceFrequency(),
  //   this will never fail on systems that run XP or later.
  //   https://msdn.microsoft.com/library/windows/desktop/ms644905.aspx
  LARGE_INTEGER perf_counter_frequency = {};
  ::QueryPerformanceFrequency(&perf_counter_frequency);
  ASSERT(perf_counter_now >= perf_counter_initial);
  uint64_t perf_counter_ticks = perf_counter_now - perf_counter_initial;
  double elapsed_time_seconds =
      perf_counter_ticks / static_cast<double>(perf_counter_frequency.QuadPart);

  const double MinimumEvaluationPeriodSeconds = 0.05;
  if (elapsed_time_seconds < MinimumEvaluationPeriodSeconds)
    return 0;

  // Compute the frequency of the TSC.
  ASSERT(tsc_now >= tsc_initial);
  uint64_t tsc_ticks = tsc_now - tsc_initial;
  tsc_ticks_per_second = tsc_ticks / elapsed_time_seconds;

  return tsc_ticks_per_second;
}

} // namespace stp
