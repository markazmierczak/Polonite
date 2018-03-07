// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Thread/NativeThread.h"

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
    ASSERT(isOk(error));
  }

  ~PthreadAttributes() {
    int error = pthread_attr_destroy(&attr_);
    ASSERT(error == 0);
  }

  void SetDetachState(bool start_detached) {
    int state = start_detached ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED;
    int error = pthread_attr_setdetachstate(&attr_, state);
    ASSERT(error == 0);
  }

  void SetStackSize(int64_t size) {
    int error = pthread_attr_setstacksize(&attr_, toUnsigned(size));
    ASSERT(error == 0, "invalid stack size for created thread");
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

Expected<NativeThread::ObjectHandlePair, ErrorCode> NativeThread::Create(
    Delegate* delegate, int64_t stack_size) {
  ASSERT(stack_size >= 0);

  PthreadAttributes attributes;
  if (stack_size > 0)
    attributes.SetStackSize(stack_size);

  pthread_t thread;
  auto error = static_cast<PosixErrorCode>(
      pthread_create(&thread, attributes.get(), ThreadFunc, static_cast<void*>(delegate)));
  if (!isOk(error))
    return makeErrorCode(error);
  return ObjectHandlePair { thread, thread };
}

Expected<int, ErrorCode> NativeThread::Join(NativeThreadObject thread) {
  void* exit_code;
  auto error = static_cast<PosixErrorCode>(pthread_join(thread, &exit_code));
  if (!isOk(error))
    return makeErrorCode(error);
  return static_cast<int>(reinterpret_cast<intptr_t>(exit_code));
}

Expected<void, ErrorCode> NativeThread::Detach(NativeThreadObject thread) {
  auto error = static_cast<PosixErrorCode>(pthread_detach(thread));
  if (!isOk(error))
    return makeErrorCode(error);
  return Expected<void, ErrorCode>();
}

void NativeThread::Yield() {
  int rv = sched_yield();
  ASSERT(rv == 0, "sched_yield failed");
}

void NativeThread::SleepFor(TimeDelta duration) {
  struct timespec sleep_time = duration.toTimespec();
  struct timespec remaining;

  while (nanosleep(&sleep_time, &remaining) == -1 && errno == EINTR)
    sleep_time = remaining;
}

void NativeThread::SleepUntil(TimeTicks end_time) {
  SleepFor(end_time - TimeTicks::Now());
}

} // namespace stp
