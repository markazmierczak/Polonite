// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_APP_ATEXIT_H_
#define STP_BASE_APP_ATEXIT_H_

#include "Base/Containers/Stack.h"
#include "Base/Thread/Lock.h"
#include "Base/Util/Function.h"

namespace stp {

class BASE_EXPORT AtExitManager {
  DISALLOW_COPY_AND_ASSIGN(AtExitManager);
 public:
  typedef Function<void()> Callback;

  AtExitManager();
  ~AtExitManager();

  static void registerCallback(void (*func)(void*), void* param);
  static void registerCallback(Callback callback);

  static void processCallbacksNow();

 protected:
  explicit AtExitManager(bool shadow);

 private:
  Lock lock_;
  Stack<Callback> stack_;
  AtExitManager* next_manager_;  // Stack of managers to allow shadowing.
  bool processing_callbacks_ = false;

  static AtExitManager* g_top_manager_;
};

#if defined(UNIT_TEST)
class ShadowingAtExitManager : public AtExitManager {
 public:
  ShadowingAtExitManager() : AtExitManager(true) {}
};
#endif // defined(UNIT_TEST)

} // namespace stp

#endif // STP_BASE_APP_ATEXIT_H_
