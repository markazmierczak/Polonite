// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_THREAD_THREADLOCAL_H_
#define STP_BASE_THREAD_THREADLOCAL_H_

#include "Base/Thread/NativeThreadLocal.h"

namespace stp {

// Thread local with destruction guarantees.
//
// TLS native implementation lacks capability for calling a function
// on thread-exit.
//
// ThreadLocalPtr class assures a destructor is called on all platforms.
//
// Note: Calling Fini() will NOT invoke any destructor.
// It is user responsibility to make sure that all threads using the variable
// have exited already.

class BASE_EXPORT BasicThreadLocal {
 public:
  void Init(void (*dtor)(void*));
  void Fini();

  void* Get() { return NativeThreadLocal::GetValue(slot_); }
  void Set(void* value);

  #if OS(WIN)
  static void OnThreadExit();
  #endif

 private:
  NativeThreadLocal::Slot slot_;
  void (*dtor_)(void*);
  #if OS(WIN)
  BasicThreadLocal* prev_;
  BasicThreadLocal* next_;
  void appendToList(BasicThreadLocal* list);
  void removeFromList();
  #endif
};

template<typename T>
class ThreadLocalPtr : public BasicThreadLocal {
 public:
  void Init(void (*dtor)(T*)) {
    BasicThreadLocal::Init(reinterpret_cast<void(*)(void*)>(dtor));
  }

  void Fini() { BasicThreadLocal::Fini(); }

  T* Get() { return static_cast<T*>(BasicThreadLocal::Get()); }
  void Set(T* value) { BasicThreadLocal::Set(value); }
};

} // namespace stp

#endif // STP_BASE_THREAD_THREADLOCAL_H_
