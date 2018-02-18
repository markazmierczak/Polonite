// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/Console.h"

#include "Base/App/Application.h"
#include "Base/Containers/ArrayOps.h"

#include <android/log.h>

namespace stp {

static android_LogPriority GetLogPriority(LogLevel log_level) {
  switch (log_level) {
    case LogLevelFATAL:
      return ANDROID_LOG_FATAL;
    case LogLevelERROR:
      return ANDROID_LOG_ERROR;
    case LogLevelWARN:
      return ANDROID_LOG_WARN;
    case LogLevelINFO:
      return ANDROID_LOG_INFO;
    case LogLevelVERBOSE:
      return ANDROID_LOG_VERBOSE;
    case LogLevelUSER:
      break;
  }
  return ANDROID_LOG_UNKNOWN;
}

void ConsoleWriter::PrintToSystemDebugLog(StringSpan text) {
  auto priority = GetLogPriority(log_level_);

  InlineString<256> text_copy(text);
  const String& app_name = Application::instance().getName();
  __android_log_write(priority, ToNullTerminated(app_name), ToNullTerminated(text_copy));
}

} // namespace stp
