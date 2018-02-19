// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Thread/ThreadLocal.h"

#include "Base/Thread/Lock.h"

namespace stp {

#if OS(WIN)
static BasicLock g_tls_lock = BASIC_LOCK_INITIALIZER;
static BasicThreadLocal g_tls_root = { 0, nullptr, &g_tls_root, &g_tls_root };
static int g_with_dtor_count = 0;

void BasicThreadLocal::appendToList(BasicThreadLocal* list) {
  this->next_ = list;
  this->prev_ = list->prev_;
  list->prev_->next_ = this;
  list->prev_ = this;
}

void BasicThreadLocal::RemoveFromList() {
  this->prev_->next_ = this->next_;
  this->next_->prev_ = this->prev_;
  this->next_ = nullptr;
  this->prev_ = nullptr;
}

void BasicThreadLocal::OnThreadExit() {
  struct Callback {
    void (*dtor)(void*);
    void* param;
  };
  List<Callback> callbacks;

  {
    AutoLock guard(&g_tls_lock);
    callbacks.ensureCapacity(g_with_dtor_count);

    // Iterate the TLS list backwards to destroy objects in reversed order
    // of registration.
    BasicThreadLocal* it = g_tls_root.prev_;
    for (; it != &g_tls_root; it = it->prev_) {
      if (it->dtor_)
        callbacks.append(Callback { it->dtor_, it->Get(); });
    }
  }

  for (const Callback& callback : callbacks) {
    if (callback.param)
      callback.dtor(callback.param);
  }
}
#endif // OS(WIN)

void BasicThreadLocal::Init(void (*dtor)(void*)) {
  dtor_ = dtor;
  slot_ = NativeThreadLocal::Allocate(dtor_);
  #if OS(WIN)
  AutoLock guard(&g_tls_lock);
  appendToList(&g_tls_root);
  if (dtor)
    ++g_with_dtor_count;
  #endif
}

void BasicThreadLocal::Fini() {
  NativeThreadLocal::Free(slot_);
  #if OS(WIN)
  AutoLock guard(&g_tls_lock);
  RemoveFromList();
  if (dtor_)
    --g_with_dtor_count;
  #endif
}

void BasicThreadLocal::Set(void* value) {
  void* old = nullptr;
  if (dtor_)
    old = Get();

  NativeThreadLocal::SetValue(slot_, value);

  if (old)
    dtor_(old);
}

} // namespace stp
