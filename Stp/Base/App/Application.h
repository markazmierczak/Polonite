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

// This class must be accessed on main thread only !
class BASE_EXPORT Application {
 public:
  enum class Phase {
    Born,
    Initializing,
    Running,
    Finalizing,
    Dead,
  };

  static Application& instance();

  explicit Application(CommandLine::Arguments arguments);
  virtual ~Application();

  void addPart(ApplicationPart* part);

  void setName(const String& name);
  const String& getName();

  void setDisplayName(const String& display_name);
  const String& getDisplayName();

  void setVersion(const Version& version);
  const Version& getVersion() const { return version_; }

  void setExitCode(int exit_code) { exit_code_ = exit_code; }
  int getExitCode() { return exit_code_; }

  Phase getPhase() const { return phase_; }

  int run(int (*main_function)());

  void init();
  void fini();

  typedef void (*TerminateHandler)();

  [[noreturn]] static void terminate();
  static TerminateHandler setTerminateHandler(TerminateHandler handler);
  static TerminateHandler getTerminateHandler();

 protected:
  virtual void onCaptureArguments(CommandLine::Arguments& arguments);

  virtual void onDidInit();
  virtual void onWillFini();

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

inline Application& Application::instance() {
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

[[noreturn]] inline void Application::terminate() {
  abort();
}

} // namespace stp

#endif // STP_BASE_APP_APPLICATION_H_
