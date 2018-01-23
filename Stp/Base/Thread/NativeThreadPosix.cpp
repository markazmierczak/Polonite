// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Thread/NativeThread.h"

#include "Base/Error/SystemException.h"
#include "Base/Time/TimeTicks.h"

#include <sched.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>

#if OS(LINUX)
# include <sys/syscall.h>
#endif

namespace stp {

namespace {

class PthreadAttributes {
 public:
  PthreadAttributes() {
    auto error = static_cast<PosixErrorCode>(pthread_attr_init(&attr_));
    if (!IsOk(error))
      throw SystemException(error);
  }

  ~PthreadAttributes() {
    int error = pthread_attr_destroy(&attr_);
    ASSERT_UNUSED(error == 0, error);
  }

  void SetDetachState(bool start_detached) {
    int state = start_detached ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED;
    int error = pthread_attr_setdetachstate(&attr_, state);
    ASSERT_UNUSED(error == 0, error);
  }

  void SetStackSize(int64_t size) {
    int error = pthread_attr_setstacksize(&attr_, ToUnsigned(size));
    ASSERT(error == 0, "invalid stack size for created thread");
    ALLOW_UNUSED_LOCAL(error);
  }

  pthread_attr_t* get() { return &attr_; }

 private:
  pthread_attr_t attr_;
};

} // namespace

NativeThreadId NativeThread::CurrentId() {
  // PThreads doesn't have the concept of a thread ID, so we have to reach down into the kernel.
  #if OS(DARWIN)
  return pthread_mach_thread_np(pthread_self());
  #elif OS(LINUX)
  return syscall(__NR_gettid);
  #elif OS(ANDROID)
  return gettid();
  #else
  # error "unknown platform"
  #endif
}

static void* ThreadFunc(void* opaque) {
  auto* delegate = static_cast<NativeThread::Delegate*>(opaque);
  intptr_t exit_code = delegate->ThreadMain();
  return reinterpret_cast<void*>(exit_code);
}

NativeThread::ObjectHandlePair NativeThread::Create(
    Delegate* delegate, int64_t stack_size) {
  ASSERT(stack_size >= 0);

  PthreadAttributes attributes;
  if (stack_size > 0)
    attributes.SetStackSize(stack_size);

  pthread_t thread;
  auto error = static_cast<PosixErrorCode>(
      pthread_create(&thread, attributes.get(), ThreadFunc, static_cast<void*>(delegate)));
  if (!IsOk(error))
    throw Exception::WithDebug(SystemException(error), "unable to create new thread");
  return ObjectHandlePair { thread, thread };
}

int NativeThread::Join(NativeThreadObject thread) {
  void* exit_code;
  auto error = static_cast<PosixErrorCode>(pthread_join(thread, &exit_code));
  if (!IsOk(error))
    throw Exception::WithDebug(SystemException(error), "unable to join thread");
  return static_cast<int>(reinterpret_cast<intptr_t>(exit_code));
}

void NativeThread::Detach(NativeThreadObject thread) {
  auto error = static_cast<PosixErrorCode>(pthread_detach(thread));
  if (!IsOk(error))
    throw Exception::WithDebug(SystemException(error), "unable to detach thread");
}

void NativeThread::Yield() {
  int rv = sched_yield();
  ASSERT(rv == 0, "sched_yield failed");
  ALLOW_UNUSED_LOCAL(rv);
}

void NativeThread::SleepFor(TimeDelta duration) {
  struct timespec sleep_time = duration.ToTimeSpec();
  struct timespec remaining;

  while (nanosleep(&sleep_time, &remaining) == -1 && errno == EINTR)
    sleep_time = remaining;
}

void NativeThread::SleepUntil(TimeTicks end_time) {
  SleepFor(end_time - TimeTicks::Now());
}

} // namespace stp
