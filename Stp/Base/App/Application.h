// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_APP_APPLICATION_H_
#define STP_BASE_APP_APPLICATION_H_

#include "Base/Process/CommandLine.h"
#include "Base/Util/Version.h"

#if OS(WIN)
#include "Base/Win/WindowsHeader.h"
#endif

namespace stp {

class ApplicationPart;
class AtExitManager;

class BASE_EXPORT Application {
 public:
  enum class Phase {
    Born,
    Initializing,
    Running,
    Finalizing,
    Dead,
  };

  static Application& Instance();

  explicit Application(CommandLine::Arguments arguments);
  virtual ~Application();

  // Registers a single part within application.
  // Must be called before application object is initialized.
  void AddPart(ApplicationPart* part);

  // ASCII encoded string.
  // Must not contain any (back)slashes or colons and be non empty.
  void SetName(StringSpan name);
  const String& GetName();

  // UTF-8 encoded.
  void SetDisplayName(StringSpan display_name);
  const String& GetDisplayName();

  void SetVersion(const Version& version);
  const Version& GetVersion() const { return version_; }

  void SetExitCode(int exit_code) { exit_code_ = exit_code; }
  int GetExitCode() { return exit_code_; }

  Phase GetPhase() const { return phase_; }

  int Run(int (*main_function)());

  void Init();
  void Fini();

  typedef void (*TerminateHandler)();

  [[noreturn]] static void Terminate();
  static TerminateHandler SetTerminateHandler(TerminateHandler handler);
  static TerminateHandler GetTerminateHandler();

 protected:
  // A way to modify arguments before they are passed to CommandLine constructor.
  virtual void OnCaptureArguments(CommandLine::Arguments& arguments);

  // Handlers called after initialization/before finalization process.
  virtual void OnDidInit();
  virtual void OnWillFini();

 private:
  String name_;

  String display_name_;

  Version version_;

  Phase phase_ = Phase::Born;

  ApplicationPart* parts_head_ = nullptr;
  ApplicationPart* parts_tail_ = nullptr;

  ApplicationPart* part_being_registered_ = nullptr;

  AtExitManager* at_exit_manager_ = nullptr;

  int exit_code_ = 0;

  CommandLine::Arguments native_arguments_;

  static Application* g_instance_;
};

inline Application& Application::Instance() {
  ASSERT(g_instance_);
  return *g_instance_;
}

#if OS(ANDROID)

#elif OS(WIN)

#if 1 // FIXME defined(WIN_CONSOLE_APP)
#define APPLICATION_MAIN() int wmain(int argc, wchar_t* argv[])
#define APPLICATION_ARGUMENTS stp::CommandLine::Arguments(argc, argv)
#else

#define APPLICATION_MAIN() \
  int APIENTRY wWinMain(HINSTANCE, HINSTANCE, wchar_t* args, int)

#define APPLICATION_ARGUMENTS stp::CommandLine::Arguments(args)

#endif // defined(WIN_CONSOLE_APP)

#elif OS(DARWIN)

// On Mac don't return from main, to avoid the apparent removal of main from
// stack backtraces under tail call optimization.
#define DEFINE_APP_MAIN(parts) \
  __attribute__((visibility("default"))) \
  int main(int argc, char* argv[])

#define APPLICATION_ARGUMENTS stp::CommandLine::Arguments(argc, argv)

#elif OS(POSIX)

#define APPLICATION_ARGUMENTS stp::CommandLine::Arguments(argc, argv)
#define APPLICATION_MAIN() int main(int argc, char* argv[])

#endif // OS(*)

} // namespace stp

#endif // STP_BASE_APP_APPLICATION_H_
