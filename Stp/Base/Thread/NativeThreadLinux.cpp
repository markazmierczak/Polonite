// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Thread/NativeThread.h"

#include "Base/Debug/Log.h"
#include "Base/Error/SystemException.h"
#include "Base/Text/FormatMany.h"

#if !OS(ANDROID)
# include <sched.h>
#endif
#if !OS(FREEBSD)
# include <sys/prctl.h>
#endif

namespace stp {

#if !OS(ANDROID)
void NativeThread::SetPriority(NativeThreadObject thread, ThreadPriority priority) {
  PosixErrorCode error;

  #ifdef SCHED_IDLE
  if (priority == ThreadPriority::Idle) {
    sched_param param = { 0 };
    error = static_cast<PosixErrorCode>(pthread_setschedparam(thread, SCHED_IDLE, &param));
    if (!IsOk(error))
      throw Exception::withDebug(SystemException(error), "unable to set idle policy for thread");
    return;
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
  error = static_cast<PosixErrorCode>(pthread_setschedparam(thread, policy, &param));
  if (!IsOk(error))
    throw Exception::withDebug(SystemException(error), "unable to set idle policy for thread");
}
#endif // OS(*)

#if !OS(FREEBSD)
void NativeThread::SetName(const String& name) {
  // On linux we can get the thread names to show up in the debugger by setting
  // the process name for the LWP. We don't want to do this for the main
  // thread because that would rename the process, causing tools like killall
  // to stop working.
  if (NativeThread::CurrentId() == getpid()) {
    LOG(ERROR, "cannot change main thread name");
    return;
  }

  // Set the name for the LWP (which gets truncated to 15 characters).
  int err = prctl(PR_SET_NAME, toNullTerminated(name));

  auto error_code = getLastPosixErrorCode();
  if (err < 0 && error_code != PosixErrorCode::OperationNotPermitted)
    LOG(ERROR, "prctl(PR_SET_NAME) failed : {}", error_code);
}
#endif // OS(*)

} // namespace stp
