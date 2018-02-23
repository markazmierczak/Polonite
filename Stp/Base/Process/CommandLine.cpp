// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Process/CommandLine.h"

#include "Base/Containers/SpanAlgo.h"
#include "Base/FileSystem/FilePath.h"
#include "Base/Text/AsciiString.h"
#include "Base/Text/StringAlgo.h"
#include "Base/Text/Wtf.h"
#include "Base/Type/FormattableToString.h"
#include "Base/Type/ParseFloat.h"
#include "Base/Type/ParseInteger.h"

#if OS(WIN)
#include <windows.h>
#include <shellapi.h>
#endif

namespace stp {

// Since we use a lazy match, make sure that longer versions (like "--") are
// listed before shorter versions (like "-") of similar prefixes.
#if OS(WIN)
// By putting slash last, we can control whether it is treaded as a switch
// value by changing the value of switch_prefix_count to be one less than
// the array size.
constexpr StringSpan SwitchPrefixes[] = {"--", "-", "/"};
#elif OS(POSIX)
// UNIXes don't use slash as a switch.
constexpr StringSpan SwitchPrefixes[] = {"--", "-"};
#endif

constexpr StringSpan SwitchTerminator = "--";

static int g_switch_prefix_count = isizeofArray(SwitchPrefixes);

CommandLine* CommandLine::g_for_current_process_ = nullptr;

CommandLine::CommandLine() {
}

CommandLine::CommandLine(const Arguments& arguments) {
  parse(arguments);
}

CommandLine::~CommandLine() {}

void CommandLine::clear() {
  program_name_.clear();
  switches_.clear();
  positionals_.clear();
}

bool CommandLine::has(StringSpan name) const {
  return tryGet(name) != nullptr;
}

const String* CommandLine::tryGet(StringSpan name) const {
  return switches_.tryGet(name);
}

const String* CommandLine::tryGetAscii(StringSpan name) const {
  const String* value = tryGet(name);
  return (value && isAscii(*value)) ? value : nullptr;
}

bool CommandLine::tryGetInt(StringSpan name, int& out_value) const {
  const String* value = tryGet(name);
  if (!value)
    return false;
  return tryParse(*value, out_value) == ParseIntegerErrorCode::Ok;
}

bool CommandLine::tryGetFloat(StringSpan name, double& out_value) const {
  const String* value = tryGet(name);
  return value ? tryParse(*value, out_value) : false;
}

bool CommandLine::tryGet(StringSpan name, FilePath& out_value) const {
  const String* value = tryGet(name);
  if (!value)
    return false;
  out_value = FilePathSpan(value->data(), value->size());
  return true;
}

bool CommandLine::equal(StringSpan name, StringSpan value) const {
  const String* real_value = tryGet(name);
  return real_value ? *real_value == value : false;
}

void CommandLine::add(String positional) {
  positionals_.add(move(positional));
}

void CommandLine::set(StringSpan switch_name, StringSpan value) {
  set(switch_name, String(value));
}

void CommandLine::set(StringSpan switch_name, String&& value) {
  switches_.set(switch_name, move(value));
}

bool CommandLine::parseSwitch(StringSpan argument, String& out_name, String& out_value) {
  // Detect prefix:
  int prefix_size = 0;
  for (int i = 0; i < g_switch_prefix_count; ++i) {
    StringSpan prefix = SwitchPrefixes[i];
    if (startsWith(argument, prefix)) {
      prefix_size = prefix.size();
      break;
    }
  }
  if (prefix_size == 0 || prefix_size == argument.size())
    return false;

  // Remove prefix:
  argument.removePrefix(prefix_size);

  // Split pair:
  int separator_pos = argument.indexOf(SwitchValueSeparator);

  if (separator_pos <= 0) {
    if (separator_pos == 0)
      return false;
    out_name = argument;
    return true;
  }
  out_name = argument.getSlice(0, separator_pos);
  out_value = argument.getSlice(separator_pos);
  return true;
}

#if OS(WIN)
void CommandLine::parseFromArgv(int argc, wchar_t** argv) {
  if (argc < 1)
    return;

  setProgramName(toString(makeSpanFromNullTerminated(argv[0])));

  bool parse_switches = true;
  for (int i = 1; i < argc; ++i) {
    String arg = toString(makeSpanFromNullTerminated(argv[i]));

    if (parse_switches && arg == SwitchTerminator) {
      parse_switches = false;
      continue;
    }

    String switch_name, switch_value;
    if (parse_switches && parseSwitch(arg, switch_name, switch_value))
      set(switch_name, move(switch_value));
    else
      add(move(arg));
  }
}

void CommandLine::parse(const Arguments& arguments) {
  if (arguments.args)
    parseFromArgs(arguments.args);
  else
    parseFromArgv(arguments.argc, arguments.argv);
}

void CommandLine::parseFromArgs(const wchar_t* args) {
  ASSERT(args);

  int argc;
  wchar_t** argv = ::CommandLineToArgvW(args, &argc);
  ASSERT(argc, "CommandLineToArgvW failed");

  parseFromArgv(argc, argv);
  ::LocalFree(argv);
}

void CommandLine::setSlashIsNotASwitch() {
  // The last switch prefix should be slash, so adjust the size to skip it.
  ASSERT(SwitchPrefixes[isizeofArray(SwitchPrefixes) - 1] == "/");
  g_switch_prefix_count = isizeofArray(SwitchPrefixes) - 1;
}
#else

void CommandLine::parseFromArgv(int argc, char** argv) {
  clear();

  if (argc < 1)
    return;

  program_name_ = makeSpanFromNullTerminated(argv[0]);

  bool parse_switches = true;
  for (int i = 1; i < argc; ++i) {
    #if HAVE_UTF8_NATIVE_VALIDATION
    auto arg = String(makeSpanFromNullTerminated(argv[i]));
    ASSERT(Utf8::Validate(arg));
    #else
    auto arg = WtfToUtf8(makeSpanFromNullTerminated(argv[i]));
    #endif

    if (parse_switches && arg == SwitchTerminator) {
      parse_switches = false;
      continue;
    }

    String switch_name, switch_value;
    if (parse_switches && parseSwitch(arg, switch_name, switch_value))
      set(switch_name, move(switch_value));
    else
      add(move(arg));
  }
}

void CommandLine::parse(const Arguments& arguments) {
  parseFromArgv(arguments.argc, arguments.argv);
}
#endif // OS(*)

void CommandLine::init(const Arguments& arguments) {
  ASSERT(!g_for_current_process_);
  g_for_current_process_ = new CommandLine(arguments);
}

void CommandLine::fini() {
  ASSERT(g_for_current_process_);
  delete g_for_current_process_;
  g_for_current_process_ = nullptr;
}

static void formatCommandLineArgument(TextWriter& out, StringSpan arg) {
  #if OS(WIN)
  // Quote a string as necessary for CommandLineToArgvW compatibility *on Windows*.
  // We follow the quoting rules of CommandLineToArgvW.
  // http://msdn.microsoft.com/en-us/library/17w5ykft.aspx
  StringSpan quotable_chars = " \\\"";
  if (arg.indexOfAny(quotable_chars) < 0) {
    // No quoting necessary.
    out << arg;
    return;
  }

  out << '"';
  for (int i = 0; i < arg.size(); ++i) {
    if (arg[i] == '\\') {
      // Find the extent of this run of backslashes.
      int start = i;
      int end = start + 1;
      for (; end < arg.size() && arg[end] == '\\'; ++end) {
      }
      int backslash_count = end - start;

      // Backslashes are escapes only if the run is followed by a double quote.
      // Since we also will end the string with a double quote, we escape for
      // either a double quote or the end of the string.
      if (end == arg.size() || arg[end] == '"') {
        // To quote, we need to output 2x as many backslashes.
        backslash_count *= 2;
      }
      for (int j = 0; j < backslash_count; ++j)
        out << '\\';

      // Advance i to one before the end to balance i++ in loop.
      i = end - 1;
    } else if (arg[i] == '"') {
      out << "\\\"";
    } else {
      out << arg[i];
    }
  }
  out << '"';
  #else
  out << arg;
  #endif // OS(*)
}

void CommandLine::formatImpl(TextWriter& out, const StringSpan& opts) const {
  bool with_program = true;
  if (!opts.isEmpty()) {
    if (opts[0] == 'L')
      with_program = false;
  }
  if (with_program)
    formatCommandLineArgument(out, program_name_);

  for (const String& positional : positionals_) {
    out << ' ';
    formatCommandLineArgument(out, positional);
  }

  for (const auto& pair : switches_) {
    out << ' ';
    out << SwitchPrefixes[0];
    out << pair.key();
    out << SwitchValueSeparator;

    formatCommandLineArgument(out, pair.value());
  }
}

String CommandLine::toArgvLine(bool with_program_name) const {
  return formattableToString(*this, with_program_name ? StringSpan() : StringSpan("L"));
}

} // namespace stp
