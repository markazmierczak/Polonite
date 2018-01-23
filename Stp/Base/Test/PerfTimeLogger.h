// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_TEST_PERFTIMELOGGER_H_
#define STP_BASE_TEST_PERFTIMELOGGER_H_

#include "Base/Containers/List.h"
#include "Base/Time/ElapsedTimer.h"

namespace stp {

// Automates calling LogPerfResult for the common case where you want
// to measure the time that something took. Call Done() when the test
// is complete if you do extra work after the test or there are stack
// objects with potentially expensive constructors. Otherwise, this
// class with automatically log on destruction.
class PerfTimeLogger {
 public:
  explicit PerfTimeLogger(String test_name);
  ~PerfTimeLogger();

  void Done();

 private:
  bool logged_;
  String test_name_;
  ElapsedTimer timer_;

  DISALLOW_COPY_AND_ASSIGN(PerfTimeLogger);
};

} // namespace stp

#endif // STP_BASE_TEST_PERFTIMELOGGER_H_
