.. Copyright 2017 Polonite Authors. All rights reserved.
   Copyright 2017 the V8 project authors. All rights reserved.
   Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

ExportTemplate
==============

``ExportTemplate.h`` header provides macros for using ``FOO_EXPORT`` macros with explicit template instantiation declarations and definitions. Generally, the ``FOO_EXPORT`` macros are used at declarations, and GCC requires them to be used at explicit instantiation declarations, but MSVC requires ``__declspec(dllexport)`` to be used at the explicit instantiation definitions instead.

Usage
-----

In a header file, write::

   extern template class EXPORT_TEMPLATE_DECLARE(FOO_EXPORT) Foo<bar>;

In a source file, write::

   template class EXPORT_TEMPLATE_DEFINE(FOO_EXPORT) Foo<bar>;

Implementation notes
--------------------

The implementation of this header uses some subtle macro semantics to detect what the provided ``FOO_EXPORT`` value was defined as and then to dispatch to appropriate macro definitions. Unfortunately, MSVC's C preprocessor is rather non-compliant and requires special care to make it work.

Issue 1.
^^^^^^^^

.. code-block:: c++

   #define F(x)
   F()

MSVC emits warning ``C4003`` ("not enough actual parameters for macro ``F``), even though it's a valid macro invocation.  This affects the macros below that take just an ``EXPORT`` parameter, because ``EXPORT`` may be empty.

As a workaround, we can add a dummy parameter and arguments::

   #define F(x,_)
   F(,)

Issue 2.
^^^^^^^^

.. code-block:: c++

   #define F(x) G##x
   #define Gj() ok
   F(j())

The correct replacement for ``F(j())`` is ``ok``, but MSVC replaces it with ``Gj()``.  As a workaround, we can pass the result to an identity macro to force MSVC to look for replacements again. (This is why ``EXPORT_TEMPLATE_STYLE_3`` exists.)
