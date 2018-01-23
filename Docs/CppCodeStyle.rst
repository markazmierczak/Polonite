.. _stp-cpp-code-style:

C++ Code Style
**************

The code style for Polonite takes much from `Google C++ Style Guide <https://google.github.io/styleguide/cppguide.html>`_.

IDE Support
===========

Use predefined formatter from ``Stp/Base/Tools/Eclipse/CppCodeStyle.xml`` in Eclipse.

Header Files
============

The ``#define`` Guard
---------------------

All header files should have ``#define`` guards to prevent multiple inclusion. The format of the symbol name should be ``<PROJECT>_<PATH>_<FILE>_H_``.

To guarantee uniqueness, they should be based on the full path in a project's source tree. For example, the file ``Foo/Bar/Baz.h`` in project *Foo* should have the following guard::

  #ifndef FOO_BAR_BAZ_H_
  #define FOO_BAR_BAZ_H_

  ...

  #endif // FOO_BAR_BAZ_H_

Names and Order of Includes
---------------------------

All of a project's header files should be listed as descendants of the project's source directory without use of directory shortcuts . (the current directory) or .. (the parent directory).

In ``Dir/Foo.cpp`` order your includes as follows:

1. ``Dir/Foo.h``
2. Your project's ``.h`` files
3. Other libraries ``.h`` files
4. C++ system headers
5. C system headers
6. Conditional includes with ``#if``\ s.

Naming
======

Naming rules are pretty arbitrary, but we feel that consistency is more important than individual preferences in this area, so regardless of whether you find them sensible or not, the rules are the rules.

General Naming Rules
--------------------

**✕ AVOID** abbreviations - names should be descriptive.

Give as descriptive a name as possible, within reason. Do not worry about saving horizontal space as it is far more important to make your code immediately understandable by a new reader. Do not use abbreviations that are ambiguous or unfamiliar to readers outside your project, and do not abbreviate by deleting letters within a word::

  // Good:
  int price_count_reader;    // No abbreviation.
  int num_errors;            // "num" is a widespread convention.
  int num_dns_connections;   // Most people know what "DNS" stands for.

  // Wrong:
  int n;                     // Meaningless.
  int nerr;                  // Ambiguous abbreviation.
  int n_comp_conns;          // Ambiguous abbreviation.
  int wgc_connections;       // Only your group knows what this stands for.
  int pc_reader;             // Lots of things can be abbreviated "pc".
  int cstmr_id;              // Deletes internal letters.

Note that certain universally-known abbreviations are OK, such as ``i`` for an iteration variable and ``T`` for a template parameter.

**✓ DO** favor readability over brevity.

The function name ``CanScrollHorizontally()`` is better than ``IsScrollableX()`` (an obscure reference to the X-axis).

**✓ DO** choose easily readable identifier names.

For example, a property named ``HorizontalAlignment`` is more English-readable than ``AlignmentHorizontal``.

**✓ DO** use semantically interesting names rather than language-specific keywords for type names.

For example, ``GetLength`` is a better name than ``GetInt``.

Capitalization
--------------

There are different ways to capitalize:

* camelCase
* PascalCase
* snake_case

Camel-casing is not used at all right now.

The PascalCasing convention capitalizes the first character of each word (**including acronyms**), as shown in the following examples:

* ``TextStream``
* ``JsonValue``

File Names
----------

Use **PascalCase** for filenames, e.g.: ``MyUsefulClass.cpp``.

Source files should end with ``.cpp`` and header files should end with ``.h``.

Type Names
----------

Use **PascalCase** for type names, e.g. ``MyUsefulClass``.

**✓ DO** name types with nouns or noun phrases.

This distinguishes type names from methods, which are named with verb phrases.

Some abstract classes may use adjective phrases, e.g. ``Serializable``.

**✓ CONSIDER** ending the name of derived classes with the name of the base class.

Variable Names
--------------

Use **snake_case** for variable names.

Private and protected members additionally have trailing underscores.

Global variables (also static variables inside classes/structs) contains ``g_`` prefix.

Example::

  static int g_shared_time_msecs = 0;

  void MyFunction() {
    int local_variable;
  }

  class MyClass {
   public:
    String public_data_member;

   private:
    int private_data_member_;

    static bool g_used_;
  };

**✓ DO** name collection properties with a plural phrase describing the items in the collection instead of using a singular phrase followed by ``*List``.

Constant Names
--------------

Variables declared ``constexpr`` or ``const``, and whose value is fixed for the duration of the program, are named with **PascalCase**. For example::

  constexpr int DaysInWeek = 7;

**✓ PREFER** ``constexpr`` to denote values evaluated at compile time and leave ``const`` keyword to denote a value that cannot be modified.

Enumerator's Value Names
------------------------

Values provided by an enumerator (for both scoped and unscoped ``enum``\ s) should be named like constants: ``EnumValue``.

The enumeration name, Color (and AlternateColor), is a type.

Example::

  enum Color {
    GreenColor,
    RedColor,
    BlueColor,
  };
  enum class AlternateColor {
    Green = 0,
    Red = 1,
    Blue = 2,
  };

Function Names
--------------

Use **PascalCase** for function names.

Use **snake_case** for properties inherent in the class.

For example the :class:`String` class has ``data()`` and ``size()`` accessors.

**✓ DO** give methods names that are verbs or verb phrases.

A function performs an action, so the name should make clear what it does.
Use ``Get*`` and ``Set*`` prefixes for getters and setters.
Examples::

  AddTableEntry()
  DeleteUrl()
  OpenFileOrDie()
  GetName()

Namespace Names
---------------

Use **snake_case** for namespace names.

**✕ DO NOT** separate logical components (like audio, video) with namespaces (as done in other programming languages).

**✓ DO** use single name for whole library/application.

**✓ DO** use ``internal`` namespace to enclose private, standalone code.

Inside Templates
----------------

**✓ DO** name template parameters with descriptive names unless a single-letter name is completely self-explanatory and a descriptive name would not add value.

**✓ CONSIDER** using ``T`` as the type parameter name for types with one single-letter type parameter.

**✓ DO** prefix descriptive type parameters with ``T``.

**✕ DO NOT** prefix value parameters.

**✓ DO** use ``T`` as prefix for constants/types/functions operating on type system (e.g. most utilities in ``Stp/Base/Type/`` do so).

Note that templates operating on type system might not result in another type but constant::

  template<typename T>
  constexpr bool TIsArray = detail::TIsArrayHelper<TRemoveCV<T>>::Value;

  template<typename T>
  constexpr int TRank = detail::TRankHelper<T>::Value;

Macro Names
-----------

**✕ AVOID** using macros.

You're not really going to define a macro, are you? If you do, they're like this: ``MY_MACRO_THAT_SCARES_SMALL_CHILDREN``.

Please see the :ref:`description of macros <stp-cpp-code-style-macros>`; in general macros should not be used. However, if they are absolutely needed, then they should be named with all capitals and underscores::

  #define ROUND(x) ...
  #define PI_ROUNDED 3.0

Classes
=======

Structs vs. Classes
-------------------

**✓ DO** use ``struct``\ s for passive objects that carry data; everything else is a ``class``.

The ``struct`` and ``class`` keywords behave almost identically in C++. We add our own semantic meanings to each keyword, so you should use the appropriate keyword for the data-type you're defining.

Structs may have associated constants, but lack any functionality other than access/setting the data members. The accessing/setting of fields is done by directly accessing the fields rather than through method invocations. Methods should not provide behavior but should only be used to set up the data members, e.g., constructor, destructor, ``Initialize()``, ``Reset()``, ``Validate()``.

**✓ DO** use ``class`` if more functionality is required.

.. _stp-cpp-code-style-macros:

Preprocessor Macros
-------------------

**✕ AVOID** defining macros, especially in headers; prefer inline functions, ``enum``\ s, and ``constexpr`` variables.

**✕ DO NOT** define macros in a ``.h`` file.

**✓ DO** ``#define`` macros right before you use them, and ``#undef`` them right after::

   #define MY_USEFUL_MACRO(x) ...

   int a = MY_USEFUL_MACRO("Text Literal");

   #undef MY_USEFUL_MACRO

**✕ DO NOT** just ``#undef`` an existing macro before replacing it with your own; instead, pick a name that's likely to be unique.

**✕ DO NOT** use macros that expand to unbalanced C++ constructs, or at least document that behavior well.

**✕ AVOID** using ``##`` to generate function/class/variable names.

**✓ MAKE** ``#if``\ s compatible with ``-Wundef`` compiler option. Use already defined macros only.

**✕ AVOID** using ``#ifdef``/``#ifndef`` - hard to detect a mistake, even harder when macro name changes.

Functions
=========

Arguments Ordering
------------------

Place arguments with following order:

* context
* inputs
* outputs
* `default <http://en.cppreference.com/w/cpp/language/default_arguments>`_

The context is like ``this`` pointer for a class. For example, see ``out`` in following function::

   void Format(TextWriter& out, const MyClass& value, const StringSpan& opts);

Some array processing functions use different order to mimic ``memcpy``::

   UninitializedCopy(int* dst, const int* src, int count);

Reference Arguments
-------------------

All input arguments passed by reference must be labeled with ``const``.

**✕ DO NOT** use pointers for output parameters.

However, there are some instances where using a pointer is preferable to reference:

* to denote nullability of argument.
* the function saves a pointer.

Formatting
==========

Line Length
-----------

Each line of text in your code should be at most 2^7 characters long.
But it is not a hard rule, use longer lines when limit is not feasible.
