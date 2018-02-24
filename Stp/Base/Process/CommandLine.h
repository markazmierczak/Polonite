// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

// This class works with command lines: building and parsing.
// Arguments with prefixes ('--', '-', and on Windows, '/') are switches.
// Switches will precede all other arguments without switch prefixes.
// Switches can optionally have values, delimited by '=', e.g., "-switch=value".
// An argument of "--" will terminate switch parsing during initialization,
// interpreting subsequent tokens as non-switch arguments, regardless of prefix.

// There is a singleton read-only CommandLine that represents the command line
// that the current process was started with.

#ifndef STP_BASE_PROCESS_COMMANDLINE_H_
#define STP_BASE_PROCESS_COMMANDLINE_H_

#include "Base/Compiler/Os.h"
#include "Base/Containers/FlatMap.h"
#include "Base/String/String.h"

namespace stp {

class FilePath;

class BASE_EXPORT CommandLine {
 public:
  struct Arguments {
    Arguments() {}

    #if OS(WIN)
    Arguments(int argc, wchar_t** argv) : argc(argc), argv(argv) {}
    explicit Arguments(wchar_t* args) : args(args) {}
    #else
    Arguments(int argc, char** argv) : argc(argc), argv(argv) {}
    #endif

    int argc = 0;

    #if OS(WIN)
    wchar_t** argv = nullptr;
    wchar_t* args = nullptr;
    #else
    char** argv = nullptr;
    #endif
  };

  static constexpr char SwitchValueSeparator = '=';

  static const CommandLine& forCurrentProcess();

  CommandLine();
  CommandLine(const Arguments& arguments);
  ~CommandLine();

  ALWAYS_INLINE const List<String>& positionals() const { return positionals_; }
  ALWAYS_INLINE List<String>& positionals() { return positionals_; }

  ALWAYS_INLINE const FlatMap<String, String>& switches() const { return switches_; }
  ALWAYS_INLINE FlatMap<String, String>& switches() { return switches_; }

  // Returns true if this command line contains the given switch.
  bool has(StringSpan name) const;
  const String* tryGet(StringSpan name) const;
  const String* tryGetAscii(StringSpan name) const;
  bool tryGetInt(StringSpan name, int& out_value) const;
  bool tryGetFloat(StringSpan name, double& out_value) const;
  bool tryGet(StringSpan name, FilePath& out_value) const;
  bool equal(StringSpan name, StringSpan value) const;

  void setProgramName(const String& name) { program_name_ = name; }
  const String& getProgramName() const { return program_name_; }

  void add(String positional);

  void set(StringSpan switch_name, StringSpan value);
  void set(StringSpan switch_name, String&& value);

  void clear();

  String toArgvLine(bool with_program_name = true) const;

  void parse(const Arguments& arguments);

  static void init(const Arguments& arguments);
  static void fini();

  #if OS(WIN)
  // By default command-line arguments beginning with slashes are treated
  // as switches on Windows, but not other platforms.
  //
  // If this behavior is inappropriate for your application, you can call this
  // function BEFORE initializing the current process' global command line
  // object and the behavior will be the same as POSIX systems (only hyphens
  // begin switches, everything else will be a positional argument).
  void setSlashIsNotASwitch();
  #endif

  friend TextWriter& operator<<(TextWriter& out, const CommandLine& x) {
    x.formatImpl(out, StringSpan()); return out;
  }
  friend void format(TextWriter& out, const CommandLine& x, const StringSpan& opts) {
    x.formatImpl(out, opts);
  }

 private:
  String program_name_;
  FlatMap<String, String> switches_;
  List<String> positionals_;

  #if OS(WIN)
  void parseFromArgv(int argc, wchar_t** argv);
  void parseFromArgs(const wchar_t* args);
  #else
  void parseFromArgv(int argc, char** argv);
  #endif

  void formatImpl(TextWriter& out, const StringSpan& opts) const;

  static CommandLine* g_for_current_process_;

  static bool parseSwitch(StringSpan argument, String& out_name, String& out_value);

  DISALLOW_COPY_AND_ASSIGN(CommandLine);
};

inline const CommandLine& CommandLine::forCurrentProcess() {
  ASSERT(g_for_current_process_);
  return *g_for_current_process_;
}

} // namespace stp

#endif // STP_BASE_PROCESS_COMMANDLINE_H_
