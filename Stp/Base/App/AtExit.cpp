// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/App/AtExit.h"

#include "Base/Util/Function.h"

namespace stp {

// Keep a stack of registered AtExitManagers. We always operate on the most
// recent, and we should never have more than one outside of testing (for a
// statically linked version of this library). Testing may use the shadow
// version of the constructor, and if we are building a dynamic library we may
// end up with multiple AtExitManagers on the same process. We don't protect
// this for thread-safe access, since it will only be modified in testing.
AtExitManager* AtExitManager::g_top_manager_ = nullptr;

AtExitManager::AtExitManager()
    : next_manager_(g_top_manager_) {
  // If multiple modules instantiate AtExitManagers they'll end up living in this
  // module... they have to coexist.
  #if !defined(COMPONENT_BUILD)
  ASSERT(!g_top_manager_);
  #endif
  g_top_manager_ = this;
}

AtExitManager::~AtExitManager() {
  ASSERT(this == g_top_manager_);

  ProcessCallbacksNow();
  g_top_manager_ = next_manager_;
}

void AtExitManager::RegisterCallback(void (*func)(void*), void* param) {
  ASSERT(func);
  RegisterCallback(Callback([func, param](){
    func(param);
  }));
}

void AtExitManager::RegisterCallback(Callback callback) {
  ASSERT(g_top_manager_ != nullptr, "tried to RegisterFunction without an AtExitManager");

  AutoLock lock(&g_top_manager_->lock_);
  ASSERT(!g_top_manager_->processing_callbacks_);
  g_top_manager_->stack_.Push(move(callback));
}

void AtExitManager::ProcessCallbacksNow() {
  ASSERT(g_top_manager_ != nullptr, "tried to ProcessCallbacksNow without an AtExitManager");

  // Callbacks may try to add new callbacks, so run them without holding |lock_|.
  // This is an error and caught by the ASSERT in RegisterCallback(), but
  // handle it gracefully in release builds so we don't deadlock.
  Stack<Callback> tasks;
  {
    AutoLock lock(&g_top_manager_->lock_);
    Swap(tasks, g_top_manager_->stack_);
    g_top_manager_->processing_callbacks_ = true;
  }

  while (!tasks.IsEmpty()) {
    Callback task = tasks.Pop();
    task();
  }

  // Expect that all callbacks have been run.
  ASSERT(g_top_manager_->stack_.IsEmpty());
}

AtExitManager::AtExitManager(bool shadow)
    : next_manager_(g_top_manager_) {
  ASSERT(shadow || !g_top_manager_);
  g_top_manager_ = this;
}

} // namespace stp
