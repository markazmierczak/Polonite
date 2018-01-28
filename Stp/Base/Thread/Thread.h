// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_THREAD_THREAD_H_
#define STP_BASE_THREAD_THREAD_H_

#include "Base/Containers/List.h"
#include "Base/Thread/NativeThread.h"
#include "Base/Thread/NativeThreadLocal.h"
#include "Base/Util/Function.h"

namespace stp {

namespace detail {

class ThreadData {
 public:
  // The callbacks will be invoked just before thread is destroyed.
  List<Function<void()>> at_exit;
  // Indicates whether this data was created through Thread spawning (false)
  // or adoption of main or external thread (true).
  bool adopted = false;

  static ThreadData* Current();
  static void Adopt();
  static void Register(ThreadData* that);
  static void Dispose(ThreadData* that);

  #if OS(WIN)
  static void OnThreadExit();
  #elif OS(POSIX)
  static void OnDestroy(void* opaque);
  #endif

  static void ClassInit();
  static void ClassFini();

  static NativeThreadLocal::Slot g_tls_for_current;
};

} // namespace detail

class BASE_EXPORT Thread : private NativeThread::Delegate {
 public:
  using Handle = NativeThreadHandle;

  Thread();
  ~Thread() override;

  // The name can be only assigned before thread is started.
  void SetName(String name);
  const String& GetName() const { return name_; }

  void SetStackSize(int64_t size);
  int64_t GetStackSize() const { return stack_size_; }

  void Start();
  int Join();

  void Detach();

  bool IsAlive() const { return native_handle_ != InvalidNativeThreadHandle; }

  bool TrySetPriority(ThreadPriority priority);
  void SetPriority(ThreadPriority priority);
  ThreadPriority GetPriority() const { return priority_; }

  NativeThreadHandle GetHandle() const { return native_handle_; }
  NativeThreadObject GetNativeObject() const { return native_object_; }

 protected:
  virtual int Main() = 0;

  int ThreadMain() final;

 private:
  friend class BaseApplicationPart;

  NativeThreadHandle native_handle_ = InvalidNativeThreadHandle;
  NativeThreadObject native_object_ = InvalidNativeThreadObject;

  String name_;
  int64_t stack_size_ = 0;
  ThreadPriority priority_ = ThreadPriority::Normal;
  detail::ThreadData data_;

  static void ClassInit();
  static void ClassFini();

  DISALLOW_COPY_AND_ASSIGN(Thread);
};

class BASE_EXPORT ThisThread {
  STATIC_ONLY(ThisThread);
 public:
  static Thread::Handle GetHandle() { return NativeThread::CurrentHandle(); }

  static void Yield() { NativeThread::Yield(); }

  static void SleepFor(TimeDelta duration) { NativeThread::SleepFor(duration); }
  static void SleepUntil(TimeTicks end_time) { NativeThread::SleepUntil(end_time); }

  static void Adopt() { detail::ThreadData::Adopt(); }

  static void AtExit(Function<void()> callback);
};

class BASE_EXPORT ThreadedFunction final : public Thread {
 public:
  explicit ThreadedFunction(Function<int()> main);
  ~ThreadedFunction() override;

 protected:
  int Main() override;

 private:
  Function<int()> main_;
};

} // namespace stp

#endif // STP_BASE_THREAD_THREAD_H_
