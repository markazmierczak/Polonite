// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_THREAD_NATIVETHREADLOCAL_H_
#define STP_BASE_THREAD_NATIVETHREADLOCAL_H_

#include "Base/Compiler/Os.h"
#include "Base/Debug/Assert.h"

#if OS(WIN)
#include "Base/Win/WindowsHeader.h"
#elif OS(POSIX)
#include <pthread.h>
#endif

namespace stp {

// WARNING: You should *NOT* be using this class directly.
// NativeThreadLocal is low-level abstraction to the OS's TLS interface,
// you should instead be using ThreadLocal.
class BASE_EXPORT NativeThreadLocal {
  STATIC_ONLY(NativeThreadLocal);
 public:
  #if OS(WIN)
  typedef DWORD Slot;
  #elif OS(POSIX)
  typedef pthread_key_t Slot;
  #endif

  #if OS(WIN)
  static Slot allocate();
  #elif OS(POSIX)
  static Slot allocate(void (*dtor)(void*) = nullptr);
  #endif

  static void deallocate(Slot key);

  static void setValue(Slot key, void* value);

  static void* getValue(Slot key);
};

inline void* NativeThreadLocal::getValue(Slot slot) {
  #if OS(WIN)
  return ::TlsGetValue(slot);
  #elif OS(POSIX)
  return pthread_getspecific(slot);
  #endif // OS(*)
}

inline void NativeThreadLocal::setValue(Slot slot, void* value) {
  #if OS(WIN)
  bool ok = ::TlsSetValue(slot, value) != 0;
  #elif OS(POSIX)
  bool ok = pthread_setspecific(slot, value) == 0;
  #endif // OS(*)
  ASSERT(ok, "failed to set TLS value");
}

} // namespace stp

#endif // STP_BASE_THREAD_NATIVETHREADLOCAL_H_
