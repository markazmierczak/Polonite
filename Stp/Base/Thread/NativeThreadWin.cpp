// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Thread/NativeThread.h"

#include "Base/Error/SystemException.h"
#include "Base/Time/TimeTicks.h"

namespace stp {

NativeThreadId NativeThread::CurrentId() {
  return ::GetCurrentThreadId();
}

NativeThreadId NativeThread::ObjectToId(NativeThreadObject object) {
  DWORD id = ::GetThreadId(object);
  if (id == 0)
    throw SystemException(GetLastPosixErrorCode());
  return id;
}

static DWORD __stdcall ThreadFunc(void* opaque) {
  auto* delegate = static_cast<NativeThread::Delegate*>(opaque);
  int exit_code = delegate->ThreadMain();
  return toUnsigned(exit_code);
}

NativeThread::ObjectHandlePair NativeThread::Create(
    Delegate* delegate, bool start_detached, int64_t stack_size) {
  DWORD flags = 0;
  if (stack_size > 0)
    flags |= STACK_SIZE_PARAM_IS_A_RESERVATION;

  // Using CreateThread here vs _beginthreadex makes thread creation a bit
  // faster and doesn't require the loader lock to be available. Our code will
  // have to work running on CreateThread() threads anyway, since we run code
  // on the Windows thread pool, etc. For some background on the difference:
  //   http://www.microsoft.com/msj/1099/win32/win321099.aspx
  NativeThreadHandle handle;
  NativeThreadObject thread = ::CreateThread(
      nullptr, stack_size, ThreadFunc, delegate, flags, &handle);

  if (thread == InvalidNativeThreadObject)
    throw SystemException(GetLastPosixErrorCode());

  if (start_detached) {
    if (!::CloseHandle(thread))
      ASSERT(false, "unable to detach thread");
  }
  return ObjectHandlePair { thread, handle };
}

int NativeThread::Join(NativeThreadObject thread) {
  ASSERT(thread != InvalidNativeThreadObject);

  DWORD rv = ::WaitForSingleObject(thread, INFINITE);
  if (rv != WAIT_OBJECT_0) {
    ASSERT(rv == WAIT_FAILED);
    throw Exception::WithDebug(SystemException(
        GetLastPosixErrorCode()), "unable to join thread");
  }

  DWORD exit_code;
  if (!::GetExitCodeThread(thread, &exit_code)) {
    throw Exception::WithDebug(SystemException(
        GetLastPosixErrorCode()), "unable to get thread's exit code");
  }

  Detach(thread);

  return static_cast<int>(exit_code);
}

void NativeThread::Detach(NativeThreadObject thread) {
  if (!::CloseHandle(thread)) {
    throw Exception::WithDebug(SystemException(
        GetLastPosixErrorCode()), "unable to close thread handle");
  }
}

void NativeThread::Yield() {
  #if OS(WIN_RT)
  ::SwitchToThread();
  #else
  ::Sleep(0);
  #endif
}

static void SleepImpl(TimeTicks now, TimeTicks end) {
  // When measured with a high resolution clock, Sleep() sometimes returns much
  // too early. We may need to call it repeatedly to get the desired duration.
  for (; now < end; now = TimeTicks::Now())
    ::Sleep(static_cast<DWORD>((end - now).InMillisecondsRoundedUp()));

}

void NativeThread::SleepFor(TimeDelta duration) {
  TimeTicks now = TimeTicks::Now();
  TimeTicks end = now + duration;
  SleepImpl(now, end);
}

void NativeThread::SleepUntil(TimeTicks end_time) {
  TimeTicks now = TimeTicks::Now();
  SleepImpl(now, end_time);
}

static int ThreadPriorityToNative(ThreadPriority priority) {
  switch (priority) {
    case ThreadPriority::Idle: return THREAD_PRIORITY_IDLE;
    case ThreadPriority::Lowest: return THREAD_PRIORITY_LOWEST;
    case ThreadPriority::BelowNormal: return THREAD_PRIORITY_BELOW_NORMAL;
    case ThreadPriority::Normal: return THREAD_PRIORITY_NORMAL;
    case ThreadPriority::AboveNormal: return THREAD_PRIORITY_ABOVE_NORMAL;
    case ThreadPriority::Highest: return THREAD_PRIORITY_HIGHEST;
    case ThreadPriority::TimeCritical: return THREAD_PRIORITY_TIME_CRITICAL;
    case ThreadPriority::RealtimeAudio: return THREAD_PRIORITY_TIME_CRITICAL;
  }
  UNREACHABLE(return THREAD_PRIORITY_ERROR_RETURN);
}

void NativeThread::SetPriority(NativeThreadObject thread, ThreadPriority priority) {
  int native_priority = ThreadPriorityToNative(priority);
  if (!::SetThreadPriority(thread, native_priority)) {
    throw Exception::WithDebug(SystemException(
        GetLastPosixErrorCode()), "unable to change thread priority");
  }
}

// The information on how to set the thread name comes from
// a MSDN article: http://msdn2.microsoft.com/en-us/library/xcb2z8hs.aspx
// This function has try handling, so it is separated out of its caller.
static void SetThreadNameImpl(NativeThreadId thread_id, const String& name) {
  constexpr DWORD VCThreadNameException = 0x406D1388;

  typedef struct tagTHREADNAME_INFO {
    DWORD dwType;  // Must be 0x1000.
    LPCSTR szName;  // Pointer to name (in user addr space).
    DWORD dwThreadID;  // Thread ID (-1=caller thread).
    DWORD dwFlags;  // Reserved for future use, must be zero.
  } THREADNAME_INFO;

  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = ToNullTerminated(name);
  info.dwThreadID = thread_id;
  info.dwFlags = 0;

  __try {
    RaiseException(VCThreadNameException, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
  } __except(EXCEPTION_EXECUTE_HANDLER) {
  }
}

void NativeThread::SetName(const String& name) {
  // The debugger needs to be around to catch the name in the exception.
  if (!::IsDebuggerPresent())
    return;

  SetThreadNameImpl(CurrentId(), name);
}

} // namespace stp
