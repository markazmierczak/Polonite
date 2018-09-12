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
#include "Base/Thread/Thread.h"
#include "Base/Time/TimeTicks.h"

namespace stp {

void BaseApplicationPart::init() {
  // FIXME auto& command_line = CommandLine::forCurrentProcess();
  // FIXME ALLOW_UNUSED_LOCAL(command_line);

  // FIXME
//  #if !defined(NDEBUG) && !OS(IOS)
//  // Will wait for 60 seconds for a debugger to come to attach to the process.
//  if (command_line.has("wait-for-debugger"))
//    Debugger::waitFor(60, true);
//  #endif

  // FIXME CpuInfo::ClassInit();
  detail::WeakReference::Flag::classInit();
  // FIXME Console::classInit();
  // FIXME InitLogging();
  TimeTicks::ClassInit();
  // FIXME Thread::ClassInit();
}

void BaseApplicationPart::fini() {
  // FIXME Thread::ClassFini();
  // FIXME Console::classFini();
}

constexpr const ApplicationPartInfo BaseApplicationPart::Info_ = makeInfo();

ApplicationPart BaseApplicationPart::Instance = APPLICATION_PART_INITIALIZER(Info_);

} // namespace stp
