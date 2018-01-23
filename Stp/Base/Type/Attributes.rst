.. _stp-base-compiler-attributes:

Attributes
==========

Several attributes are not yet in standard (as of C++14) but available in all compilers.

`Unused` Attributes
-------------------

``ALLOW_UNUSED_LOCAL(x)`` - Annotate a variable indicating it's OK if the variable is not used. Typically used to silence a compiler warning when the assignment is important for some other reason. Use like::

  int x = ...;
  ALLOW_UNUSED_LOCAL(x);

``ALLOW_UNUSED_TYPE`` - Annotate a typedef or function indicating it's OK if it's not used. Use like::

  typedef Foo Bar ALLOW_UNUSED_TYPE;

  int MyFunction() ALLOW_UNUSED_TYPE {
    ...
  }

``WARN_UNUSED_RESULT`` - Annotate a function indicating the caller must examine the return value. Use like::

  int foo() WARN_UNUSED_RESULT

``IgnoreResult(x)`` - Used to explicitly mark the return value of a function as unused. If you are really sure you don't want to do anything with the return value of a function that has been marked ``WARN_UNUSED_RESULT``, wrap it with this. Example::

  OwnPtr<MyType> my_var = ...;
  if (TakeOwnership(my_var.get()) == Success)
    IgnoreResult(my_var.release());

`Inline` Attributes
-------------------

``NEVER_INLINE`` - Annotate a function indicating it should not be inlined. Use like::

  NEVER_INLINE void DoStuff() { ... }

Alignment Attributes
--------------------

``ALIGNAS(x)`` - Specify memory alignment for structures, classes, etc. Use like::

  class ALIGNAS(16) MyClass { ... }
  ALIGNAS(16) int array[4];

In most places you can use the C++11 keyword ``alignas()``, which is preferred.
But compilers have trouble mixing ``__attribute__((...))`` syntax with ``alignas(...)`` syntax.

* Doesn't work in clang or gcc::

   struct alignas(16) __attribute__((packed)) S { char c; };

* Works in clang but not gcc::

   struct __attribute__((packed)) alignas(16) S2 { char c; };

* Works in clang and gcc::

   struct alignas(16) S3 { char c; } __attribute__((packed));

There are also some attributes that must be specified **before** a class definition: visibility (used for exporting functions/classes) is one of  these attributes. This means that it is not possible to use ``alignas()`` with a class that is marked as exported.

Optimizer Attributes
--------------------

``BUILTIN_UNREACHABLE()`` - Expands to an expression which states that it is undefined behavior for the compiler to reach this point. Part of internal implementation; **use** ``UNREACHABLE()`` instead.

``BUILTIN_ASSUME(cond)`` - The optimizer assumes that the condition represented by expression is true at the point where the keyword appears and remains true until expression is modified. Part of internal implementation; **use** ``ASSUME(cond)`` instead.
MSVC documentation: https://docs.microsoft.com/en-us/cpp/intrinsics/assume

``LIKELY(x)`` - Macro for hinting that an expression is likely to be true.

``UNLIKELY(x)`` - Macro for hinting that an expression is likely to be false.

``RESTRICT`` - annotate that a variable is `NOT` aliased in the current scope.

MSVC documentation: https://docs.microsoft.com/en-us/cpp/cpp/extension-restrict
