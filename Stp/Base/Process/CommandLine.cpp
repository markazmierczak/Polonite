// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Process/CommandLine.h"

#include "Base/Containers/ContiguousAlgo.h"
#include "Base/FileSystem/FilePath.h"
#include "Base/Text/Format.h"
#include "Base/Type/ParseFloat.h"
#include "Base/Text/Wtf.h"
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

static int g_switch_prefix_count = ArraySizeOf(SwitchPrefixes);

CommandLine* CommandLine::g_for_current_process_ = nullptr;

CommandLine::CommandLine() {
}

CommandLine::CommandLine(const Arguments& arguments) {
  Parse(arguments);
}

CommandLine::~CommandLine() {}

void CommandLine::Clear() {
  program_name_.Clear();
  switches_.Clear();
  positionals_.Clear();
}

bool CommandLine::Has(StringSpan name) const {
  return TryGet(name) != nullptr;
}

const String* CommandLine::TryGet(StringSpan name) const {
  return switches_.TryGet(name);
}

const String* CommandLine::TryGetAscii(StringSpan name) const {
  const String* value = TryGet(name);
  return (value && IsAscii(*value)) ? value : nullptr;
}

bool CommandLine::TryGetInt(StringSpan name, int& out_value) const {
  const String* value = TryGet(name);
  if (!value)
    return false;
  return TryParse(*value, out_value) == ParseIntegerErrorCode::Ok;
}

bool CommandLine::TryGetFloat(StringSpan name, double& out_value) const {
  const String* value = TryGet(name);
  return value ? TryParse(*value, out_value) : false;
}

bool CommandLine::TryGet(StringSpan name, FilePath& out_value) const {
  const String* value = TryGet(name);
  if (!value)
    return false;
  out_value = FilePathSpan(value->data(), value->size());
  return true;
}

bool CommandLine::Equals(StringSpan name, StringSpan value) const {
  const String* real_value = TryGet(name);
  return real_value ? *real_value == value : false;
}

void CommandLine::Add(String positional) {
  positionals_.Add(Move(positional));
}

void CommandLine::Set(StringSpan switch_name, StringSpan value) {
  Set(switch_name, String(value));
}

void CommandLine::Set(StringSpan switch_name, String&& value) {
  switches_.Set(switch_name, Move(value));
}

bool CommandLine::ParseSwitch(StringSpan argument, String& out_name, String& out_value) {
  // Detect prefix:
  int prefix_size = 0;
  for (int i = 0; i < g_switch_prefix_count; ++i) {
    StringSpan prefix = SwitchPrefixes[i];
    if (StartsWith(argument, prefix)) {
      prefix_size = prefix.size();
      break;
    }
  }
  if (prefix_size == 0 || prefix_size == argument.size())
    return false;

  // Remove prefix:
  argument.RemovePrefix(prefix_size);

  // Split pair:
  int separator_pos = argument.IndexOf(SwitchValueSeparator);

  if (separator_pos <= 0) {
    if (separator_pos == 0)
      return false;
    out_name = argument;
    return true;
  }
  out_name = argument.GetSlice(0, separator_pos);
  out_value = argument.GetSlice(separator_pos);
  return true;
}

#if OS(WIN)
void CommandLine::ParseFromArgv(int argc, wchar_t** argv) {
  if (argc < 1)
    return;

  SetProgramName(ToString(MakeSpanFromNullTerminated(argv[0])));

  bool parse_switches = true;
  for (int i = 1; i < argc; ++i) {
    String arg = ToString(MakeSpanFromNullTerminated(argv[i]));

    if (parse_switches && arg == SwitchTerminator) {
      parse_switches = false;
      continue;
    }

    String switch_name, switch_value;
    if (parse_switches && ParseSwitch(arg, switch_name, switch_value))
      Set(switch_name, Move(switch_value));
    else
      Add(Move(arg));
  }
}

void CommandLine::Parse(const Arguments& arguments) {
  if (arguments.args)
    ParseFromArgs(arguments.args);
  else
    ParseFromArgv(arguments.argc, arguments.argv);
}

void CommandLine::ParseFromArgs(const wchar_t* args) {
  ASSERT(args);

  int argc;
  wchar_t** argv = ::CommandLineToArgvW(args, &argc);
  ASSERT(argc, "CommandLineToArgvW failed");

  ParseFromArgv(argc, argv);
  ::LocalFree(argv);
}

void CommandLine::SetSlashIsNotASwitch() {
  // The last switch prefix should be slash, so adjust the size to skip it.
  ASSERT(SwitchPrefixes[ArraySizeOf(SwitchPrefixes) - 1] == "/");
  g_switch_prefix_count = ArraySizeOf(SwitchPrefixes) - 1;
}
#else

void CommandLine::ParseFromArgv(int argc, char** argv) {
  Clear();

  if (argc < 1)
    return;

  program_name_ = MakeSpanFromNullTerminated(argv[0]);

  bool parse_switches = true;
  for (int i = 1; i < argc; ++i) {
    #if HAVE_UTF8_NATIVE_VALIDATION
    auto arg = String(MakeSpanFromNullTerminated(argv[i]));
    ASSERT(Utf8::Validate(arg));
    #else
    auto arg = WtfToUtf8(MakeSpanFromNullTerminated(argv[i]));
    #endif

    if (parse_switches && arg == SwitchTerminator) {
      parse_switches = false;
      continue;
    }

    String switch_name, switch_value;
    if (parse_switches && ParseSwitch(arg, switch_name, switch_value))
      Set(switch_name, Move(switch_value));
    else
      Add(Move(arg));
  }
}

void CommandLine::Parse(const Arguments& arguments) {
  ParseFromArgv(arguments.argc, arguments.argv);
}
#endif // OS(*)

void CommandLine::Init(const Arguments& arguments) {
  ASSERT(!g_for_current_process_);
  g_for_current_process_ = new CommandLine(arguments);
}

void CommandLine::Fini() {
  ASSERT(g_for_current_process_);
  delete g_for_current_process_;
  g_for_current_process_ = nullptr;
}

static void FormatCommandLineArgument(TextWriter& out, StringSpan arg) {
  #if OS(WIN)
  // Quote a string as necessary for CommandLineToArgvW compatibility *on Windows*.
  // We follow the quoting rules of CommandLineToArgvW.
  // http://msdn.microsoft.com/en-us/library/17w5ykft.aspx
  StringSpan quotable_chars = " \\\"";
  if (arg.IndexOfAny(quotable_chars) < 0) {
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

void CommandLine::FormatImpl(TextWriter& out, const StringSpan& opts) const {
  bool with_program = true;
  if (!opts.IsEmpty()) {
    if (opts[0] == 'L')
      with_program = false;
  }
  if (with_program)
    FormatCommandLineArgument(out, program_name_);

  for (const String& positional : positionals_) {
    out << ' ';
    FormatCommandLineArgument(out, positional);
  }

  for (const auto& pair : switches_) {
    out << ' ';
    out << SwitchPrefixes[0];
    out << pair.key();
    out << SwitchValueSeparator;

    FormatCommandLineArgument(out, pair.value());
  }
}

String CommandLine::ToArgvLine(bool with_program_name) const {
  return FormattableToString(*this, with_program_name ? StringSpan() : StringSpan("L"));
}

} // namespace stp
