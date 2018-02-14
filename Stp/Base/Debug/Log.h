// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_DEBUG_LOG_H_
#define STP_BASE_DEBUG_LOG_H_

#include "Base/Text/FormatManyFwd.h"
#include "Base/Text/StringSpan.h"

namespace stp {

enum LogLevel : int {
  LogLevelFATAL = -1,
  LogLevelERROR,
  LogLevelWARN,
  LogLevelINFO,
  LogLevelVERBOSE,
  LogLevelUSER,
};

BASE_EXPORT void InitLogging();

BASE_EXPORT int VerboseLogGetLevel(const char* file);

BASE_EXPORT TextWriter* LogPrintCommon(
    LogLevel level, const char* file, unsigned line);

BASE_EXPORT void LogWrapUp(TextWriter& out);

BASE_EXPORT void LogPrint(
    LogLevel level, const char* file, unsigned line, const char* msg);

// Include FormatMany.h when used.
template<typename... Ts>
inline void LogPrint(
    LogLevel level,
    const char* file, unsigned line,
    StringSpan format, const Ts&... args) {
  TextWriter* out = LogPrintCommon(level, file, line);
  if (out) {
    FormatMany(*out, format, args...);
    LogWrapUp(*out);
  }
}

} // namespace stp

#define RELEASE_LOG(level, format, ...) \
  stp::LogPrint(LogLevel##level, __FILE__, __LINE__, format, ##__VA_ARGS__)

#if !defined(NDEBUG)
# define LOG RELEASE_LOG
# define VERBOSE_LOG(level, format, ...) \
   ((level <= stp::VerboseLogGetLevel(__FILE__)) \
     ? stp::LogPrint(LogLevelVERBOSE, __FILE__, __LINE__, format, ##__VA_ARGS__) \
     : static_cast<void>(0))
#else
# define LOG(format, ...) static_cast<void>(0)
# define VERBOSE_LOG(level, format, ...) static_cast<void>(0)
#endif

#endif // STP_BASE_DEBUG_LOG_H_
