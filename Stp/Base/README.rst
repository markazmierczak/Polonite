.. _stp-base:

Base
****

.. toctree::
   :hidden:

   App/Application.rst
   Util/Call.rst
   Containers/Containers.rst
   Compiler/Attributes.rst
   Compiler/TargetDetection.rst
   Io/Format.rst
   Json/Json.rst
   Mem/WeakPtr.rst

``App/``
========

Application lifetime management. ``ApplicationPart`` enables interconnecting several modules into one application. Each part can modify initialization/finalization sequence.

See :ref:`stp-base-app` for details.

``Util/``
=========

With ``Delegate`` one can create a direct function call to class member with devirtualization support. Especially usable in performance-wise problems.

A container for polymorphic observers ``ObserverList``.

See :ref:`stp-base-call` for details.

``Compiler/``
=============

Detection of target platform at compile time (see :ref:`base-compiler-target-detection`):

* compiler type (GCC vs MSVC) and version
* operating system
* used sanitizers
* SIMD level

Extra language attributes (see :ref:`stp-base-compiler-attributes`).

``Containers/``
===============

Container classes and working with ranges (see :ref:`stp-base-containers`).

``Debug/``
==========

Assertions and logging.

Sample call chain with ``StackTrace``.

Annotate code line with ``SourceLocation``.

``Dtoa/``
==========

Double conversion. Efficient conversion routines that have been extracted
from the V8 JavaScript engine. The code has been refactored and improved so that
it can be used more easily in other projects.

``FileSystem/``
=======

Native representation of file paths.

File streams.

File and directory manipulation: move, replace, rename, enumerate, create temporary.

Mapping file to memory with ``MemoryMappedFile``.

Infrastructure for paths management to enable fast retrieval of known locations (like executable filename, home directory).

``Geometry/``
=============

Basic math constructs (like angle, vector), affine and 3D transformations, shapes.

``Crypto/``
=========

Introduce ``Hashable`` concept.

Hashing algorithms: MD5, SHA1, CRC32.

``Io/``
=======

Binary and text streams.

Text formatting with ``Formattable`` concept.

``Json/``
=========

Working with JSON values, serialization and deserialization.
See :ref:`stp-base-json` for more.

``Math/``
=========

C math wrappers.

Bit manipulation.

Safe conversions between numeric types.

``Half`` precision floating-point type.

``Fixed<P>`` point number.

Checked/overflow/saturated mathematics.

``Mem/``
========

Smart pointers: ``OwnPtr<>``, ``RefPtr<>`` and :ref:`WeakPtr <stp-base-weak-ptr>`.

Allocators: basic, aligned, contiguous.

``Process/``
============

``CommandLine`` parsing and formatting.

Right now ``NativeProcess`` only, but in future full process management: launching, IPC, etc.

``Random/``
===========

Pseudo and cryptographic-safe random generators.

``Thread/``
=========

Atomic operations.

Synchronization between threads with locks and ``WaitableEvent``.

``System/``
===========

Runtime detection of system capabilities (``CpuInfo`` and ``SysInfo``).

Shared library loading (``Library``).

``Environment`` management.

``Test/``
=========

GTest/GMock and PerfTest.

``Text/``
=========

Polonite has its own string classes. Fast concatenation of multiple strings::

  return String::ConcatArgs(name, decorator, ".dll");

Check if your string ends with some other string::

  if (str.EndsWith("xyz"))

Simple string splitting::

  List<String> components = StringSpan("abc-def-xyz").SplitToStrings('-');

More functionality for string manipulation is available including: searching, replacing, trimming, counting.
Small string optimization is used on all platforms.

Unicode support.

Fast print/parse for numeric types.

Universal codec system for transcoding on fly.

``Thread/``
===========

Thread creation/management. Variables local to the thread (TLS).

Check if correct thread access your code with ``ThreadChecker``.

``Time/``
=========

Duration, wall clock, monotonic clock and more.

``Type/``
=========

Basic concepts and type system routines (``<type_traits>`` replacement).

Get the name of your type with ``TGetName()``.

Cast objects safely with ``IsInstanceOf()`` and ``ObjectCast()``.

``Util/``
=========

A collection of some generic classes useful in other components.
