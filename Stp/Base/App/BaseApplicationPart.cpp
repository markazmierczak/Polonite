// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/App/BaseApplicationPart.h"

#include "Base/App/Application.h"
#include "Base/Debug/Console.h"
#include "Base/Debug/Debugger.h"
#include "Base/Debug/Log.h"
#include "Base/Memory/WeakPtr.h"
#include "Base/Process/CommandLine.h"
#include "Base/System/CpuInfo.h"
#include "Base/Text/FormatMany.h"
#include "Base/Thread/Thread.h"
#include "Base/Time/TimeTicks.h"

namespace stp {

void BaseApplicationPart::Init() {
  auto& command_line = CommandLine::ForCurrentProcess();
  ALLOW_UNUSED_LOCAL(command_line);

  #if !defined(NDEBUG) && !OS(IOS)
  // Will wait for 60 seconds for a debugger to come to attach to the process.
  if (command_line.Has("wait-for-debugger"))
    Debugger::WaitFor(60, true);
  #endif

  CpuInfo::ClassInit();
  detail::WeakReference::Flag::ClassInit();
  Console::ClassInit();
  InitLogging();
  TimeTicks::ClassInit();
  Thread::ClassInit();
}

void BaseApplicationPart::Fini() {
  Thread::ClassFini();
  Console::ClassFini();
}

constexpr const ApplicationPartInfo BaseApplicationPart::Info_ = MakeInfo();

ApplicationPart BaseApplicationPart::Instance = APPLICATION_PART_INITIALIZER(Info_);

} // namespace stp
