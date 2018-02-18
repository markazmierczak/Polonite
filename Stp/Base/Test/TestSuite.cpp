// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Test/TestSuite.h"

#include "Base/App/Application.h"
#include "Base/Debug/Debugger.h"
#include "Base/Test/GMock.h"
#include "Base/Test/GTest.h"
#include "Base/Time/Time.h"

namespace stp {

namespace {

bool IsMarkedMaybe(const testing::TestInfo& test) {
  return strncmp(test.name(), "MAYBE_", 6) == 0;
}

class MaybeTestDisabler : public testing::EmptyTestEventListener {
 public:
  void OnTestStart(const testing::TestInfo& test_info) override {
    ASSERT_FALSE(IsMarkedMaybe(test_info))
        << "Probably the OS #ifdefs don't include all of the necessary "
           "platforms.\nPlease ensure that no tests have the MAYBE_ prefix "
           "after the code is preprocessed.";
  }
};

void CatchMaybeTests() {
  testing::TestEventListeners& listeners = testing::UnitTest::GetInstance()->listeners();
  listeners.Append(new MaybeTestDisabler);
}

void SuppressErrorDialogs() {
  #if OS(WIN)
  UINT new_flags = SEM_FAILCRITICALERRORS |
                   SEM_NOGPFAULTERRORBOX |
                   SEM_NOOPENFILEERRORBOX;

  // Preserve existing error mode, as discussed at
  // http://blogs.msdn.com/oldnewthing/archive/2004/07/27/198410.aspx
  UINT existing_flags = SetErrorMode(new_flags);
  SetErrorMode(existing_flags | new_flags);

  #if defined(_DEBUG) && defined(_HAS_EXCEPTIONS) && (_HAS_EXCEPTIONS == 1)
  // Suppress the "Debug Assertion Failed" dialog.
  // http://groups.google.com/d/topic/googletestframework/OjuwNlXy5ac/discussion
  _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
  _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
  #endif // defined(_DEBUG) && defined(_HAS_EXCEPTIONS) && (_HAS_EXCEPTIONS == 1)
  #endif // OS(WIN)
}

} // namespace

void TestSuite::onCaptureArguments(CommandLine::Arguments& arguments) {
  #if OS(WIN)
  testing::GTEST_FLAG(catch_exceptions) = false;
  #endif

  testing::InitGoogleTest(&arguments.argc, arguments.argv);
  testing::InitGoogleMock(&arguments.argc, arguments.argv);
}

void TestSuite::onDidInit() {
  #if OS(IOS)
  InitIOSTestMessageLoop();
  #endif

  #if OS(ANDROID)
  InitAndroidTest();
  #endif

  // In some cases, we do not want to see standard error dialogs.
  if (!Debugger::IsPresent()) {
    SuppressErrorDialogs();
    Debugger::SetSuppressDebugUI(true);
  }
  CatchMaybeTests();
}

int TestSuite::Run() {
  init();

  #if OS(IOS)
  RunTestsFromIOSApp();
  #endif

  #if OS(DARWIN)
  mac::ScopedNSAutoreleasePool scoped_pool;
  #endif

  int result = RUN_ALL_TESTS();
  setExitCode(result);

  fini();
  return result;
}

} // namespace stp
