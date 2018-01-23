// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Debug/Log.h"

#include "Base/Containers/List.h"
#include "Base/Debug/Console.h"
#include "Base/Process/CommandLine.h"
#include "Base/Type/Formattable.h"
#include "Base/Type/ParseInteger.h"
#include "Base/Util/LazyInstance.h"

namespace stp {

// Gives the default maximal active logging verbosity; 0 is the default.
// Normally positive values are used.
// --v switch controls this value.
static int g_max_log_level = 0;
static const bool g_location_is_printed = false;

struct VmoduleMatcher {
  enum MatchTarget { MatchModule, MatchFile };

  explicit VmoduleMatcher(String pattern_)
      : pattern(Move(pattern_)),
        level(2) {
    // If the pattern contains a separator, we assume that
    // it's meant to be tested against the entire __FILE__ string.
    match_target = IndexOfAny(pattern, "\\/") < 0 ? MatchModule : MatchFile;
  }

  String pattern;
  int level;
  MatchTarget match_target;
};

static LazyInstance<List<VmoduleMatcher>>::LeakAtExit g_verbose_matchers = LAZY_INSTANCE_INITIALIZER;

void LogWrapUp(TextWriter& out_base) {
  ConsoleWriter& out = static_cast<ConsoleWriter&>(out_base);
  out << EndOfLine;
  out.SetLogLevel(LogLevelUSER);
}

static void PrintLogLevel(ConsoleWriter& out, StringSpan name, ConsoleColor color) {
  out.SetForegroundColor(color);
  out.WriteAscii(name);
  out.ResetColors();
}

TextWriter* LogPrintCommon(LogLevel level, const char* file, unsigned line) {
  if (level > g_max_log_level) {
    // Max log level variable does not control VERBOSE logging.
    if (level != LogLevelVERBOSE)
      return nullptr;
  }

  ConsoleWriter& out = level <= LogLevelERROR ? Console::Err() : Console::Out();

  out.Write('[');

  switch (level) {
    case LogLevelERROR:
      PrintLogLevel(out, "ERROR", ConsoleColor::Red);
      break;
    case LogLevelWARN:
      PrintLogLevel(out, "WARN", ConsoleColor::Yellow);
      break;
    case LogLevelINFO:
      PrintLogLevel(out, "INFO", ConsoleColor::Blue);
      break;
    case LogLevelVERBOSE:
      PrintLogLevel(out, "VERBOSE", ConsoleColor::DarkBlue);
      break;

    case LogLevelFATAL: // FATAL level is reserved for assertions.
    case LogLevelUSER: // USER is special level to denote no logging in ConsoleWriter.
      UNREACHABLE();
  }

  out.WriteAscii("] ");

  if (g_location_is_printed) {
    out << MakeSpanFromNullTerminated(file) << ':' << line << ' ';
  }

  // Level is restored on exit from LOG() call.
  out.SetLogLevel(level);

  return &out;
}

void LogPrint(LogLevel level, const char* file, unsigned line, const char* msg) {
  TextWriter* out = LogPrintCommon(level, file, line);
  if (!out)
    return;

  *out << MakeSpanFromNullTerminated(msg);

  LogWrapUp(*out);
}

// Given a path, returns the basename with the extension chopped off
// We avoid using FilePath to minimize the number of dependencies the logging system has.
static StringSpan GetModule(StringSpan file) {
  StringSpan module = file;

  int last_slash_pos = LastIndexOfAny(module, "\\/");
  if (last_slash_pos >= 0)
    module.RemovePrefix(last_slash_pos + 1);

  int extension_start = module.LastIndexOf('.');
  if (extension_start >= 0)
    module.Truncate(extension_start);

  return module;
}

static bool MatchVlogPattern(StringSpan string, StringSpan pattern) {
  StringSpan p = pattern;
  StringSpan s = string;
  // Consume characters until the next star.
  while (!p.IsEmpty() && !s.IsEmpty() && p[0] != '*') {
    switch (p[0]) {
      // A slash (forward or back) must match a slash (forward or back).
      case '/':
      case '\\':
        if ((s[0] != '/') && (s[0] != '\\'))
          return false;
        break;

      // A '?' matches anything.
      case '?':
        break;

      // Anything else must match literally.
      default:
        if (p[0] != s[0])
          return false;
        break;
    }
    p.RemovePrefix(1);
    s.RemovePrefix(1);
  }

  // An empty pattern here matches only an empty string.
  if (p.IsEmpty())
    return s.IsEmpty();

  // Coalesce runs of consecutive stars. There should be at least one.
  while (!p.IsEmpty() && p[0] == '*')
    p.RemovePrefix(1);

  // Since we moved past the stars, an empty pattern here matches anything.
  if (p.IsEmpty())
    return true;

  // Since we moved past the stars and p is non-empty, if some
  // non-empty substring of s matches p, then we ourselves match.
  while (!s.IsEmpty()) {
    if (MatchVlogPattern(s, p))
      return true;
    s.RemovePrefix(1);
  }

  // Otherwise, we couldn't find a match.
  return false;
}

int VerboseLogGetLevel(const char* file_cstr) {
  auto file = MakeSpanFromNullTerminated(file_cstr);
  if (!g_verbose_matchers->IsEmpty()) {
    for (const auto& matcher : *g_verbose_matchers) {
      bool match_file = matcher.match_target == VmoduleMatcher::MatchFile;
      StringSpan target = match_file ? file : GetModule(file);
      if (MatchVlogPattern(target, matcher.pattern))
        return matcher.level;
    }
  }
  return g_max_log_level;
}

// Parses the per-module maximal logging levels to override the value
// given by --v.
// "--vmodule=my_module=2,foo*=3" would change the logging
// level for all code in source files "my_module.*" and "foo*.*".
//
// Any pattern containing a forward or backward slash will be tested
// against the whole pathname and not just the module, e.g.:
// "*/foo/bar/*=2" would change the logging level for all code in
// source files under a "foo/bar" directory.
static void ParseMatchers(StringSpan input) {
  List<VmoduleMatcher>* matchers = g_verbose_matchers.Pointer();

  while (!input.IsEmpty()) {
    int comma = input.IndexOf(',');
    StringSpan pair = input.GetSlice(0, comma);

    bool parsed = false;

    int pos = pair.LastIndexOf('=');
    if (pos >= 0) {
      VmoduleMatcher matcher(String(pair.GetSlice(0, pos)));
      if (TryParse(pair.GetSlice(pos + 1), matcher.level) == ParseIntegerErrorCode::Ok) {
        matchers->Add(Move(matcher));
        parsed = true;
      }
    }
    if (!parsed) {
      TextWriter* out = LogPrintCommon(LogLevelERROR, "", 0);
      if (out) {
        out->WriteAscii("unable to parse vmodule: ");
        out->Write(pair);
        *out << EndOfLine;
        LogWrapUp(*out);
      }
    }
  }
}

void InitLogging() {
  auto& command_line = CommandLine::ForCurrentProcess();

  const String* v = command_line.TryGet("v");
  if (v) {
    if (TryParse(*v, g_max_log_level) != ParseIntegerErrorCode::Ok) {
      g_max_log_level = 0;
      LOG(ERROR, "unable to parse --v switch");
    }
  }

  const String* vmodule = command_line.TryGet("v");
  if (vmodule)
    ParseMatchers(*vmodule);
}

} // namespace stp
