// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_THREAD_NATIVETHREAD_H_
#define STP_BASE_THREAD_NATIVETHREAD_H_

#include "Base/Compiler/Os.h"
#include "Base/Time/TimeTicks.h"

#if OS(WIN)
#include "Base/Win/WindowsHeader.h"
#elif OS(DARWIN)
#include <mach/mach_types.h>
#endif

#if OS(POSIX)
#include <pthread.h>
#include <unistd.h>
#endif

namespace stp {

class String;

enum class ThreadPriority : int {
  Idle,
  Lowest,        // Suitable for threads that shouldn't disrupt high priority work.
  BelowNormal,
  Normal,        // Default priority level.
  AboveNormal,   // Suitable for threads which generate data for the display (at ~60Hz).
  Highest,
  TimeCritical,

  // Priorities with special meaning:
  RealtimeAudio, // Suitable for low-latency, glitch-resistant audio.
};

// Used for thread checking and debugging.
// Meant to be as fast as possible.
// These are produced by NativeThread::CurrentHandle(), and used to later
// check if we are on the same thread or not by using ==. These are safe
// to copy between threads, but can't be copied to another process as they
// have no meaning there. Also, the handle can be re-used after a thread dies,
// so a NativeThreadHandle cannot be reliably used to distinguish a new thread
// from an old, dead thread.
#if OS(WIN)
typedef DWORD NativeThreadHandle;
#elif OS(POSIX)
typedef pthread_t NativeThreadHandle;
#endif
constexpr NativeThreadHandle InvalidNativeThreadHandle = 0;

// Used to operate on threads.
// On Windows, this is a pseudo handle constant which will always represent
// the thread using it and hence should not be shared with other threads nor
// be used to differentiate the current thread from another.
#if OS(WIN)
typedef HANDLE NativeThreadObject;
constexpr NativeThreadObject InvalidNativeThreadObject = nullptr;
#elif OS(POSIX)
typedef pthread_t NativeThreadObject;
constexpr NativeThreadObject InvalidNativeThreadObject = 0;
#endif

// Used for logging. Always an integer value.
// On POSIX systems (where pthreads are used) NativeThreadHandle is not an ID
// presented in debugger.
#if OS(WIN)
typedef DWORD NativeThreadId;
#elif OS(DARWIN)
typedef mach_port_t NativeThreadId;
#elif OS(POSIX)
typedef pid_t NativeThreadId;
#endif
constexpr NativeThreadId InvalidNativeThreadId = 0;

class BASE_EXPORT NativeThread {
  STATIC_ONLY(NativeThread);
 public:
  // Implement this interface to run code on a background thread.
  // Your ThreadMain method will be called on the newly created thread.
  class BASE_EXPORT Delegate {
   public:
    virtual int ThreadMain() = 0;

   protected:
    virtual ~Delegate() {}
  };

  // Gets the current thread handle.
  // Can be used to check if we're on the right thread quickly.
  static NativeThreadHandle currentHandle();

  // Gets the current thread ID, which may be useful for logging purposes.
  static NativeThreadId CurrentId();

  // Get the object representing the current thread.
  static NativeThreadObject CurrentObject();

  // Creates a new thread.
  //
  // The |stack_size| parameter can be 0 to indicate that the default stack size should be used.
  // Upon success, new thread reference will be returned,
  // and |delegate|'s ThreadMain method will be executed on the newly created thread.
  //
  // NOTE: When you are done with the thread reference, you must call Join to
  // release system resources associated with the thread.
  //
  // You must ensure that the Delegate object outlives the thread.
  struct ObjectHandlePair {
    NativeThreadObject object;
    NativeThreadHandle handle;
  };
  static ObjectHandlePair Create(Delegate* delegate, int64_t stack_size = 0);

  // Joins with a thread created via the Create function.
  // This function blocks the caller until the designated thread exits.
  // This will invalidate |thread|.
  static int Join(NativeThreadObject thread);

  // Detaches and releases the thread reference.
  // The thread is no longer joinable and |thread| is invalidated after this call.
  static void Detach(NativeThreadObject thread);

  // Yield the current thread so another thread can be scheduled.
  static void Yield();

  // Sleeps for the specified duration.
  static void SleepFor(TimeDelta duration);
  static void SleepUntil(TimeTicks end_time);

  // It is advised to use "try" variant. The "throw" variant may fail
  // for many reasons and it should be non-fatal in majority of cases.
  static void SetPriority(NativeThreadObject thread, ThreadPriority priority);

  // Sets the thread name visible to debuggers/tools.
  static void SetName(const String& name);

  // Not all platforms support NativeThreadObject -> NativeThreadId conversion.
  #if !OS(LINUX)
  static NativeThreadId ObjectToId(NativeThreadObject object);
  #endif
};

inline NativeThreadHandle NativeThread::currentHandle() {
  #if OS(WIN)
  return ::GetCurrentThreadId();
  #elif OS(POSIX)
  return ::pthread_self();
  #endif
}

inline NativeThreadObject NativeThread::CurrentObject() {
  #if OS(WIN)
  return ::GetCurrentThread();
  #elif OS(POSIX)
  return ::pthread_self();
  #endif
}

#if OS(DARWIN) || OS(ANDROID)
inline NativeThreadId NativeThread::ObjectToId(NativeThreadObject object) {
  #if OS(DARWIN)
  return pthread_mach_thread_np(object);
  #else OS(ANDROID)
  return pthread_getthreadid_np(object);
  #endif
}
#endif

} // namespace stp

#endif // STP_BASE_THREAD_NATIVETHREAD_H_
