// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Thread/NativeThread.h"

#include "Base/Android/JniAndroid.h"
#include "Base/Compiler/Sanitizer.h"
#include "jni/ThreadUtils_jni.h"

namespace stp {

static int ToNativePriority(ThreadPriority priority) {
  // https://developer.android.com/reference/android/os/Process.html
  switch (priority) {
    case ThreadPriority::Idle: return 19;
    case ThreadPriority::Lowest: return 15;
    case ThreadPriority::BelowNormal: return 10;
    case ThreadPriority::Normal: return 0;
    case ThreadPriority::AboveNormal: return -10;
    case ThreadPriority::Highest: return -15;
    case ThreadPriority::TimeCritical: return -19;

    case ThreadPriority::RealtimeAudio: return -16;
  }
  UNREACHABLE(return 0);
}

void NativeThread::SetPriority(NativeThreadObject thread, ThreadPriority priority) {
  NativeThreadId tid = ObjectToId(thread);
  int native_priority = ToNativePriority(priority);

  JNIEnv* env = android::AttachCurrentThread();
  Java_ThreadUtils_setThreadPriority(env, tid, native_priority);
}

void NativeThread::ThreadExit() {
  android::DetachFromVM();
}

} // namespace stp
