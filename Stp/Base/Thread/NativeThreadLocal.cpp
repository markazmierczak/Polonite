// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Thread/NativeThreadLocal.h"

#include "Base/Error/SystemException.h"

namespace stp {

#if OS(WIN)
NativeThreadLocal::Slot NativeThreadLocal::Allocate() {
  Slot slot = ::TlsAlloc();
  if (slot == TLS_OUT_OF_INDEXES)
    throw Exception::With(Exception(), "run out of TLS indices");
  return slot;
}

void NativeThreadLocal::Free(Slot slot) {
  if (!::TlsFree(slot))
    ASSERT(false, "failed to deallocate TLS slot");
}

#elif OS(POSIX)
NativeThreadLocal::Slot NativeThreadLocal::Allocate(void (*dtor)(void*)) {
  Slot slot;
  auto error = static_cast<PosixErrorCode>(pthread_key_create(&slot, dtor));
  if (!IsOk(error))
    throw Exception::With(SystemException(error), "run out of TLS indices");
  return slot;
}

void NativeThreadLocal::Free(Slot slot) {
  if (pthread_key_delete(slot) != 0)
    ASSERT(false, "failed to deallocate TLS slot");
}
#endif // OS(*)

} // namespace stp