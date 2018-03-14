// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Thread/NativeThread.h"

#include "Base/String/StringSpan.h"

#if !OS(ANDROID)
# include <sched.h>
#endif
#if !OS(FREEBSD)
# include <sys/prctl.h>
#endif

namespace stp {

#if !OS(ANDROID)
ErrorCode NativeThread::SetPriority(NativeThreadObject thread, ThreadPriority priority) {
  #ifdef SCHED_IDLE
  if (priority == ThreadPriority::Idle) {
    sched_param param = { 0 };
    ErrorCode error = static_cast<PosixErrorCode>(
        pthread_setschedparam(thread, SCHED_IDLE, &param));
    if (!isOk(error)) {
      // Unable to set idle policy for thread.
      return error;
    }
    return ErrorCode();
  }
  #endif

  if (priority == ThreadPriority::RealtimeAudio)
    priority = ThreadPriority::TimeCritical;

  int policy = SCHED_RR;
  int min = sched_get_priority_min(policy);
  int max = sched_get_priority_max(policy);
  ASSERT(min != -1 && max != -1);

  constexpr int MaxPriority = static_cast<int>(ThreadPriority::TimeCritical);
  ASSERT(static_cast<int>(priority) <= MaxPriority);

  int p = min + (max - min) * static_cast<int>(priority) / MaxPriority;

  sched_param param = { p };
  return static_cast<PosixErrorCode>(pthread_setschedparam(thread, policy, &param));
}
#endif // OS(*)

#if !OS(FREEBSD)
ErrorCode NativeThread::SetName(const char* name_cstr) {
  // From spec:
  //  The name can be up to 16 bytes long, including the terminating null byte.
  //  (If the length of the string, including the terminating null byte,
  //  exceeds 16 bytes, the string is silently truncated.)
  //
  // Sometimes the name of thread begins with organization prefix, like:
  //   org.polonite.MyThread
  //
  constexpr int MaxNameLength = 16 - 1; // 1 for null character
  auto name = StringSpan::fromCString(name_cstr);
  if (name.length() > MaxNameLength) {
    int dot_index = name.lastIndexOf('.');
    if (dot_index >= 0)
      name.removePrefix(dot_index + 1);
  }
  ASSERT(*(name.data() + name.length()) == '\0');

  int rv = prctl(PR_SET_NAME, name.data());
  if (rv != 0)
    return getLastPosixErrorCode();

  return ErrorCode();
}
#endif // OS(*)

} // namespace stp
