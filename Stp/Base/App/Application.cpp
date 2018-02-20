// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/App/Application.h"

#include "Base/App/AtExit.h"
#include "Base/App/BaseApplicationPart.h"
#include "Base/Compiler/Sanitizer.h"
#include "Base/Debug/Console.h"
#include "Base/Debug/StackTrace.h"
#include "Base/Error/ExceptionPtr.h"
#include "Base/FileSystem/KnownPaths.h"
#include "Base/Text/AsciiString.h"
#include "Base/Text/FormatMany.h"

namespace stp {

static BasicLock g_data_lock = BASIC_LOCK_INITIALIZER;

Application* Application::g_instance_ = nullptr;

static void defaultTerminate();

static inline void appendPartToList(ApplicationPart*& tail, ApplicationPart* part) {
  tail->next_ = part;
  part->prev_ = tail;
  tail = part;
}

/**
 * Register a single @a part within application.
 * Must be called before application object is initialized.
 */
void Application::addPart(ApplicationPart* part) {
  ASSERT(part != nullptr);
  ASSERT(phase_ == Phase::Born);

  if (part->status_ != ApplicationPart::Unregistered) {
    ASSERT(part->status_ == ApplicationPart::Registered,
           "application parts are circular dependent {}<->{}",
           part->getName(), part_being_registered_->getName());
    return; // already registered
  }

  // mark the part is going to be registered..
  part->status_ = ApplicationPart::Registering;
  part_being_registered_ = part;

  // resolve dependencies.
  auto deps = part->getDependencies();
  if (deps) {
    for (; *deps; ++deps)
      addPart(*deps);
  }

  part_being_registered_ = nullptr;
  part->status_ = ApplicationPart::Registered;

  appendPartToList(parts_tail_, part);
}

Application::Application(CommandLine::Arguments native_arguments)
    : native_arguments_(native_arguments) {
  at_exit_manager_ = new AtExitManager();

  // Don't create this pool on iOS, since the outer loop is already handled
  // and a loop that's destroyed in shutdown interleaves badly with the event
  // loop pool on iOS.
  #if OS(MAC)
  // We need this pool for all the objects created before we get to the
  // event loop, but we don't want to leave them hanging around until the
  // app quits. Each "main" needs to flush this pool right before it goes into
  // its main event loop to get rid of the cruft.
  NSAutoreleasePoolInit();
  #endif

  parts_head_ = &BaseApplicationPart::Instance;
  parts_tail_ = parts_head_;

  #ifndef OFFICIAL_BUILD
  // Print stack traces to stderr when crashes occur. This opens up security
  // holes so it should never be enabled for official builds.
  // FIXME StackTrace::EnableInProcessDump();
  #if OS(WIN)
  RouteStdioToConsole(false);
  ::LoadLibraryA("dbghelp.dll");
  #endif

  #endif
}

Application::~Application() {}

int Application::run(int (*main_function)()) {
  init();
  setExitCode(main_function());
  fini();
  return exit_code_;
}

void Application::init() {
  ASSERT(phase_ == Phase::Born);

  phase_ = Phase::Initializing;

  setTerminateHandler(defaultTerminate);

  onCaptureArguments(native_arguments_);

  CommandLine::Init(native_arguments_);

  for (auto* part = parts_head_; part; part = part->next_)
    part->init();

  onDidInit();

  phase_ = Phase::Running;
}

void Application::fini() {
  ASSERT(phase_ == Phase::Running);

  phase_ = Phase::Finalizing;

  onWillFini();

  for (auto* part = parts_tail_; part; part = part->prev_)
    part->fini();

  CommandLine::Fini();

  #if OS(MAC)
  NSAutoreleasePoolFini();
  #endif

  #if OS(WIN)
  # ifdef _CRTDBG_MAP_ALLOC
  _CrtDumpMemoryLeaks();
  # endif // _CRTDBG_MAP_ALLOC
  #endif // OS(WIN)

  #if SANITIZER(LEAK)
  // Invoke leak detection now, to avoid dealing with shutdown-only leaks.
  // If leaks are found, the process will exit here.
  __lsan_do_leak_check();
  #endif

  delete at_exit_manager_;

  phase_ = Phase::Dead;
}

/**
 * @param name
 * ASCII encoded string.
 * Must not contain any (back)slashes or colons and be non empty.
 */
void Application::setName(StringSpan name) {
  ASSERT(!name.isEmpty());
  // Many clients depends on short name being ASCII.
  ASSERT(isAscii(name));
  // Short name may not contain, otherwise it cannot be used in path specification.
  // FIXME ASSERT(name.indexOfAny("/\\:") < 0);

  ASSERT(phase_ == Phase::Born);
  name_ = name;
}

static String ResolveNameFromExecutablePath() {
  // FIXME
  return String("toReplace");
}

const String& Application::getName() {
  AutoLock auto_lock(&g_data_lock);
  if (name_.isEmpty())
    name_ = ResolveNameFromExecutablePath();
  return name_;
}

/**
 * @param display_name
 * UTF-8 encoded.
 */
void Application::setDisplayName(StringSpan display_name) {
  ASSERT(!display_name.isEmpty());

  ASSERT(phase_ == Phase::Born);
  display_name_ = display_name;
}

const String& Application::getDisplayName() {
  return !display_name_.isEmpty() ? display_name_ : getName();
}

void Application::setVersion(const Version& version) {
  ASSERT(phase_ == Phase::Born);
  version_ = version;
}

void Application::onDidInit(){}
void Application::onWillFini() {}
/**
 * Override to modify @a arguments before they are passed to @ref CommandLine constructor.
 */
void Application::onCaptureArguments(CommandLine::Arguments& arguments) {}

static void defaultTerminate() {
  auto exception_ptr = ExceptionPtr::Current();
  if (exception_ptr) {
    try {
      exception_ptr.Rethrow();
    } catch(Exception& exception) {
      RELEASE_LOG(FATAL, "unhandled exception {}", exception);
    } catch(...) {
      RELEASE_LOG(FATAL, "unhandled unknown exception");
    }
  } else {
    RELEASE_LOG(FATAL, "terminated");
  }
}

[[noreturn]] void Application::terminate() {
  std::terminate();
  abort();
}

Application::TerminateHandler Application::setTerminateHandler(TerminateHandler handler) {
  return std::set_terminate(handler);
}

Application::TerminateHandler Application::getTerminateHandler() {
  return std::get_terminate();
}

} // namespace stp
