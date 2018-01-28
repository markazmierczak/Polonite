// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Thread/Thread.h"

#include "Base/Thread/Lock.h"
#include "Base/Thread/ThreadLocal.h"

namespace stp {

namespace detail {

NativeThreadLocal::Slot ThreadData::g_tls_for_current;

ThreadData* ThreadData::Current() {
  return static_cast<ThreadData*>(NativeThreadLocal::GetValue(g_tls_for_current));
}

void ThreadData::Register(ThreadData* that) {
  NativeThreadLocal::SetValue(g_tls_for_current, that);
}

void ThreadData::Adopt() {
  ThreadData* that = Current();
  if (that)
    return; // already adopted

  that = new ThreadData();
  that->adopted = true;
  Register(that);
}

void ThreadData::Dispose(ThreadData* that) {
  ASSERT(that);
  List<Function<void()>> at_exit = Move(that->at_exit);

  // Invoke callbacks in reverse order.
  for (int i = at_exit.size() - 1; i >= 0; --i) {
    auto& callback = at_exit[i];
    callback();
  }

  if (that->adopted)
    delete that;
}

#if OS(WIN)
void ThreadData::OnThreadExit() {
  ThreadData* that = Current();
  if (that)
    Dispose(that);
}
#elif OS(POSIX)
void ThreadData::OnDestroy(void* opaque) {
  Dispose(static_cast<ThreadData*>(opaque));
}
#endif

void ThreadData::ClassInit() {
  #if OS(WIN)
  g_tls_for_current = NativeThreadLocal::Allocate();
  #elif OS(POSIX)
  g_tls_for_current = NativeThreadLocal::Allocate(OnDestroy);
  #endif
}

void ThreadData::ClassFini() {
  // Dispose main thread data.
  Dispose(Current());

  NativeThreadLocal::Free(g_tls_for_current);
}

} // namespace detail

Thread::Thread() {
}

Thread::~Thread() {
  ASSERT(!IsAlive());
}

void Thread::SetName(String name) {
  ASSERT(!IsAlive());
  name_ = Move(name);
}

void Thread::SetStackSize(int64_t size) {
  ASSERT(!IsAlive());
  stack_size_ = size;
}

bool Thread::TrySetPriority(ThreadPriority priority) {
  try {
    SetPriority(priority);
  } catch(...) {
    return false;
  }
  return true;
}

void Thread::SetPriority(ThreadPriority priority) {
  ASSERT(IsAlive());
  if (priority_ == priority)
    return;
  NativeThread::SetPriority(native_object_, priority);
  priority_ = priority;
}

void Thread::Start() {
  ASSERT(!IsAlive());
  auto rv = NativeThread::Create(this, stack_size_);
  native_object_ = rv.object;
  native_handle_ = rv.handle;
}

int Thread::Join() {
  ASSERT(IsAlive());
  ASSERT(GetHandle() != ThisThread::GetHandle(), "tried to join itself");

  int exit_code = NativeThread::Join(native_object_);
  native_object_ = InvalidNativeThreadObject;
  native_handle_ = InvalidNativeThreadHandle;
  return exit_code;
}

void Thread::Detach() {
  NativeThread::Detach(native_object_);
  native_object_ = InvalidNativeThreadObject;
  native_handle_ = InvalidNativeThreadHandle;
}

int Thread::ThreadMain() {
  detail::ThreadData::Register(&data_);

  if (!name_.IsEmpty())
    NativeThread::SetName(name_);

  return Main();
}

void ThisThread::AtExit(Function<void()> callback) {
  auto* data = detail::ThreadData::Current();
  ASSERT(data, "thread needs to be adopted to use this function");
  data->at_exit.Add(Move(callback));
}

ThreadedFunction::ThreadedFunction(Function<int()> main)
    : main_(Move(main)) {
}

ThreadedFunction::~ThreadedFunction() {
}

int ThreadedFunction::Main() {
  return main_();
}

void Thread::ClassInit() {
  detail::ThreadData::ClassInit();
  // Adopt main thread.
  ThisThread::Adopt();
}

void Thread::ClassFini() {
  detail::ThreadData::ClassFini();
}

} // namespace stp
