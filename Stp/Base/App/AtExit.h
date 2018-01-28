// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_APP_ATEXIT_H_
#define STP_BASE_APP_ATEXIT_H_

#include "Base/Containers/Stack.h"
#include "Base/Thread/Lock.h"
#include "Base/Util/FunctionFwd.h"

namespace stp {

class BASE_EXPORT AtExitManager {
 public:
  typedef Function<void()> Callback;

  AtExitManager();
  ~AtExitManager();

  static void RegisterCallback(void (*func)(void*), void* param);

  static void RegisterCallback(Callback callback);

  // Calls the registered callbacks in LIFO order.
  // It is possible to register new callbacks after calling this function.
  static void ProcessCallbacksNow();

 protected:
  // This constructor will allow this instance of AtExitManager to be created
  // even if one already exists. This should only be used for testing!
  // AtExitManagers are kept on a global stack, and it will be removed during
  // destruction.  This allows you to shadow another AtExitManager.
  explicit AtExitManager(bool shadow);

 private:
  Lock lock_;
  Stack<Callback> stack_;
  AtExitManager* next_manager_;  // Stack of managers to allow shadowing.
  bool processing_callbacks_ = false;

  static AtExitManager* g_top_manager_;

  DISALLOW_COPY_AND_ASSIGN(AtExitManager);
};

#if defined(UNIT_TEST)
class ShadowingAtExitManager : public AtExitManager {
 public:
  ShadowingAtExitManager() : AtExitManager(true) {}
};
#endif // defined(UNIT_TEST)

} // namespace stp

#endif // STP_BASE_APP_ATEXIT_H_
