// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Thread/NativeThread.h"

#include "Base/mac/foundation_util.h"
#include "Base/mac/mach_logging.h"

#import <Foundation/Foundation.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach/thread_policy.h>
#include <sys/resource.h>

namespace stp {

// If Cocoa is to be used on more than one thread, it must know that the
// application is multithreaded.  Since it's possible to enter Cocoa code
// from threads created by pthread_thread_create, Cocoa won't necessarily
// be aware that the application is multithreaded.  Spawning an NSThread is
// enough to get Cocoa to set up for multithreaded operation, so this is done
// if necessary before pthread_thread_create spawns any threads.
//
// http://developer.apple.com/documentation/Cocoa/Conceptual/Multithreading/CreatingThreads/chapter_4_section_4.html
void NativeThread::Init() {
  static BOOL multithreaded = [NSThread isMultiThreaded];
  if (!multithreaded) {
    // +[NSObject class] is idempotent.
    [NSThread detachNewThreadSelector:@selector(class)
                             toTarget:[NSObject class]
                           withObject:nil];
    multithreaded = YES;

    ASSERT([NSThread isMultiThreaded]);
  }
}

void NativeThread::SetName(const char* name) {
  pthread_setname_np(name);
}

static void SetPriorityNormal(mach_port_t mach_thread_id) {
  // Make thread standard policy.
  // Please note that this call could fail in rare cases depending
  // on runtime conditions.
  thread_standard_policy policy;
  kern_return_t result = thread_policy_set(mach_thread_id,
                                           THREAD_STANDARD_POLICY,
                                           reinterpret_cast<thread_policy_t>(&policy),
                                           THREAD_STANDARD_POLICY_COUNT);

  if (result != KERN_SUCCESS)
    MACH_DVLOG(1, result) << "thread_policy_set";
}

// Enables time-contraint policy and priority suitable for low-latency,
// glitch-resistant audio.
static void SetPriorityRealtimeAudio(mach_port_t mach_thread_id) {
  // Increase thread priority to real-time.

  // Please note that the thread_policy_set() calls may fail in
  // rare cases if the kernel decides the system is under heavy load
  // and is unable to handle boosting the thread priority.
  // In these cases we just return early and go on with life.

  // Make thread fixed priority.
  thread_extended_policy_data_t policy;
  policy.timeshare = 0;  // Set to 1 for a non-fixed thread.
  kern_return_t result = thread_policy_set(mach_thread_id,
                                           THREAD_EXTENDED_POLICY,
                                           reinterpret_cast<thread_policy_t>(&policy),
                                           THREAD_EXTENDED_POLICY_COUNT);
  if (result != KERN_SUCCESS) {
    MACH_DVLOG(1, result) << "thread_policy_set";
    return;
  }

  // Set to relatively high priority.
  thread_precedence_policy_data_t precedence;
  precedence.importance = 63;
  result = thread_policy_set(mach_thread_id,
                             THREAD_PRECEDENCE_POLICY,
                             reinterpret_cast<thread_policy_t>(&precedence),
                             THREAD_PRECEDENCE_POLICY_COUNT);
  if (result != KERN_SUCCESS) {
    MACH_DVLOG(1, result) << "thread_policy_set";
    return;
  }

  // Most important, set real-time constraints.

  // Define the guaranteed and max fraction of time for the audio thread.
  // These "duty cycle" values can range from 0 to 1.  A value of 0.5
  // means the scheduler would give half the time to the thread.
  // These values have empirically been found to yield good behavior.
  // Good means that audio performance is high and other threads won't starve.
  const double GuaranteedAudioDutyCycle = 0.75;
  const double MaxAudioDutyCycle = 0.85;

  // Define constants determining how much time the audio thread can
  // use in a given time quantum.  All times are in milliseconds.

  // About 128 frames @44.1KHz
  const double TimeQuantum = 2.9;

  // Time guaranteed each quantum.
  const double AudioTimeNeeded = GuaranteedAudioDutyCycle * TimeQuantum;

  // Maximum time each quantum.
  const double MaxTimeAllowed = MaxAudioDutyCycle * TimeQuantum;

  // Get the conversion factor from milliseconds to absolute time
  // which is what the time-constraints call needs.
  mach_timebase_info_data_t tb_info;
  mach_timebase_info(&tb_info);
  double ms_to_abs_time =
      (static_cast<double>(tb_info.denom) / tb_info.numer) * 1000000;

  thread_time_constraint_policy_data_t time_constraints;
  time_constraints.period = TimeQuantum * ms_to_abs_time;
  time_constraints.computation = AudioTimeNeeded * ms_to_abs_time;
  time_constraints.constraint = MaxTimeAllowed * ms_to_abs_time;
  time_constraints.preemptible = 0;

  result = thread_policy_set(mach_thread_id,
                             THREAD_TIME_CONSTRAINT_POLICY,
                             reinterpret_cast<thread_policy_t>(&time_constraints),
                             THREAD_TIME_CONSTRAINT_POLICY_COUNT);
  MACH_DVLOG_IF(1, result != KERN_SUCCESS, result) << "thread_policy_set";
  return;
}

void NativeThread::SetPriority(ThreadPriority priority) {
  // Convert from pthread_t to mach thread identifier.
  mach_port_t mach_thread_id = CurrentID();

  switch (priority) {
    case ThreadPriority::Normal:
    case ThreadPriority::Background:
    case ThreadPriority::Display:
      SetPriorityNormal(mach_thread_id);
      break;

    case ThreadPriority::RealtimeAudio:
      SetPriorityRealtimeAudio(mach_thread_id);
      break;
  }

} // namespace stp
