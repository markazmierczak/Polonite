.. _stp-base-app:

Application & Main
******************

Application
===========

Platforms has different definition of main function:

* The standard is ``main(int args, char* argv[])``.
* On Windows we have many signatures: ``wmain``, ``wWinMain``.
* On Android the application starts in Java code and there is no notion of ``main`` function.

Two macros (``APPLICATION_MAIN`` and ``APPLICATION_ARGUMENTS``) are provided to mitigate the differences between ``main`` signatures::

  int MyMain() {
    ...
  }

  APPLICATION_MAIN() {
    stp::Application app(APPLICATION_ARGUMENTS);
    return app.Run(&MyMain);
  }

The ``Application`` class manages application lifetime, initializes ``CommandLine`` instance and provides basic information about an application.

An instance of ``Application`` class **must** always be created. The standard way is to create an instance on stack of ``main`` function. One might create the instance of ``Application`` in custom way (when integrating with other libraries), using following steps:

* Create an object (on heap or stack of main function) of an ``Application`` class (or subclass).
* Add all of required ``ApplicationPart`` your application depends on.
* Use ``Application::Init()`` when application starts, i.e. before using any function from library. This will initialize command line and all ``ApplicationPart``\ s.
* Use ``Application::Fini()`` when application exits. It will finalize all parts.

Application Name
================

An ``Application`` has two name properties: `short` and `display` name.
`short` name is used when communicating with system code or creating application-dependent file paths, .e.g.: ``~/.config/short_name``.
The `display` name on the other hand is used when communicating directly with the user.

Change application name before ``Application::Init()`` is called.
If not provided, short and display names are resolved from executable file name.

Application Parts
-----------------

The application can be composed of many individual pieces of code/libraries, one library dependent on many others.

The ``Application`` class provides a way to initialize and finalize all these pieces by simple interface called ``ApplicationPart``.
An ``ApplicationPart`` subclass may modify initialization and finalization process. A graph of parts is created. All dependents of the part are initialized before and finalized after the handlers provided by the part. A part can register its dependencies by implementing ``ApplicationPart::Register()``.

.. warning:: The graph of ``ApplicationPart``\ s must be directed. Any cycles are detected in debug builds.

.. tip:: The **Base** component is always initialized before any other parts and does `NOT` need to be added as dependency.

Each part is initialized/finalized exactly once, even a part is dependency of many. The parts are finalized in reversed order, so dependencies are in valid state when a part is finalized.

Following example shows how to create a subclass for GStreamer::

  // GStreamerAppPart.h file

  #include "Base/App/ApplicationPart.h"

  #include <gst/gst.h>

  class GStreamerAppPart : public ApplicationPart {
   public:
    static GStreamerAppPart* Get();

    void Init() override;
    void Fini() override;
    StringSpan GetName() override;
  };

  // GStreamerAppPart.cpp file

  void GStreamerAppPart::Init() {
    gst_init();
  }

  void GStreamerAppPart::Fini() {
    gst_deinit();
  }

  StringSpan GStreamerAppPart::GetName() {
    return "GStreamer";
  }

  GStreamerAppPart* GStreamerAppPart::Get() {
    DEFINE_STATIC_LOCAL(GStreamerAppPart, g_instance, ());
    return &g_instance;
  }

All parts are used from main thread.
As the result one can use ``DEFINE_STATIC_LOCAL()`` to define a singleton for an application part. The singleton can be referenced by other parts then.

AtExit
------

An ``Application`` object creates ``AtExitManager`` instance.
The ``AtExitManager`` class provides a facility similar to the CRT ``atexit()``, except that we control when the callbacks are executed. Under Windows for a DLL they happen at a really bad time and under the loader lock.

When the exit manager object goes out of scope, all the registered callbacks and singleton destructors will be called.

Register a callback with ``AtExitManager::RegisterFunction()`` to be called when application dies.
