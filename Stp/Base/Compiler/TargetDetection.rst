.. _base-compiler-target-detection:

Target Detection
================

Every cross-platform application must know on what platform it is running (or even compiled for) at some point.

Some features are target dependent. The availability of a feature can be checked at either **compile time** or **runtime**. Some features can be checked only partially at compile time. E.g.: SIMD level can be set through compiler options, but often minimum possible setting is chosen (SSE2 by default for Polonite) to make application run on older systems. The availability of more recent SIMD version (e.g. SSE4 or AVX) can be detected at runtime only. The pattern is to prepare code for better CPUs, detect what features are available when application starts, and enable efficient variants.

This document presents compile time only detection.
To know about runtime detection see ``Stp/Base/System``.

Operating System
----------------

``OS(name)`` macro evaluates to true if target OS matches ``name``. OSs are grouped together to ease selection when many provides similar functionality:

* ``POSIX`` - currently all except those in ``WIN``
* ``DARWIN`` - ``MAC``, ``IOS`` and ``TVOS``
* ``WIN`` - Windows based

.. note:: that ``OS(LINUX)`` does *NOT* match ``OS(ANDROID)`` since the two differs too much.

Example::

  #include "Base/Compiler/Os.h"

  #if OS(WIN)
  # include <windows.h>
  #elif OS(POSIX)
  # include <pthread.h>
  #else
  # error "unsupported OS"
  #endif // OS(*)

CPU
---

The CPU detection can be divided into: a family and 32/64 bit architecture.

* To detect a family use ``CPU(X86_FAMILY)``/``CPU(ARM_FAMILY)``
* To detect number of bits use ``CPU(64BIT)``/``CPU(32BIT)``.

The two can be combined::

  #include "Base/Compiler/Cpu.h"

  #if CPU(ARM64)
  // ... code for 64-bit ARM
  #elif CPU(X86_32)
  // ... code for 32-bit X86
  #endif

CPU SIMD
--------

SSE/AVX/NEON availability can be checked at compile time. Default options of Polonite assumes SSE2 on x86 processors and NEON on ARM processors. Example::

  #include "Base/Compiler/Os.h"

  #if CPU_SIMD(SSE2)
  // ... SSE2 code ...
  #elif CPU_SIMD(NEON)
  // ... NEON code ...
  #endif

The headers needed to write SIMD code are automatically included (e.g. ``<emmintrin.h>`` for SSE2 and ``<arm_neon.h>`` for NEON).

.. note:: ``CPU_SIMD()`` detects minimum available version. Use :class:`CpuInfo` to detect real SIMD version available at runtime.

Compiler
--------

Use ``COMPILER(name)`` to detect which compiler is used to compile your code.

Clang compiler can pretend to be GCC or MSVC.

Example::

  #include "Base/Compiler/Config.h"

  #if COMPILER(GCC) && !COMPILER(CLANG)
  // ... code for GCC only
  #endif

Sanitizers
----------

Detect whether a sanitizer is used during compilation::

  #if SANITIZER(ADDRESS)
  # error "will not compile with ASan"
  #endif

Following macros are defined:

* ``SANITIZER(ANY)`` - if any of following
* ``SANITIZER(ADDRESS)`` for ASan
* ``SANITIZER(LEAK)`` for LSan

See ``Base/BUILD.gn`` file for complete list of available sanitizer flags.
