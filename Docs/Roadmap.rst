.. _stp-roadmap:

Roadmap
*******

Just a short list of what is buzzing in our heads.

Base Component
==============

There are still many things to do and test within the fundamental component.

**Stability** improvements. More, more, more **Tests**.

**Comprehensive** documentation.

Finish **Io**:

* ``TextReader`` with support for codecs.
* ``BinaryReader``/``BinaryWriter`` to transform your types into binary form.

Finish **Fs**:
* FileSystem Watchers
* Lexicographic comparison (system-dependent)

**Regex** - support for regular expressions

Tighter **GTest**/**GMock** integration:

* Use of Polonite classes instead of ``std``.
* Use ``IsNear()`` to handle ``EXPECT_NEAR``.
* Use ``Formattable`` concept within GTest.

**Android**/**iOS**/**Mac** support.

More **Containers** - ``Deque``, ``BitList``, ``DenseMap``, ``CircularBuffer``.

Support for **debugging**:

* Debug Visualizers for Visual Studio.
* Python scripts for GDB.

**Compress** utilities.

**Function** as ``std::function`` replacement.

**Tasks** for scheduling and cooperative multitasking with fibers.

Utilize system-level calls to provide well-defined interfaces for asynchronous programming: Message Loop and Timers.

**Process** management - launch, IPC, etc.

**Internationalization** with ICU.

**Minimal Example**

A repository with code presenting a preferred way to use the Polonite within external projects.

New Components
==============

**Audio** for streaming to/from audio hardware.

**Ui** component with ability to interact with Windowing System and GPU (OpenGL/DirectX/Vulkan).

**Net** for sockets.

**Vfs** component to access and create compressed resources or drive images.

**Gfx** - render your content.

* Integration of Skia for graphics.
* Compositor with modern renderer to enable 3D user interface and fancy effects.
