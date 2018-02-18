.. _stp-docs-std-library:

Standard C++ Library
********************

This documents describes some of differences between Polonite and Standard C++ Library. Only selected differences are presented.

Formatting
==========

Standard C++ Library use ``operator<<`` for text output.
This has many drawbacks, not limited to:

* stateful - the way the argument is formatted depends on current state::

   out << std::setbase(16);
   // From now on, all integers are printed with hexadecimal format.

* locale-dependent
* poor extensibility

The Polonite comes with ``Formattable`` concept which is built upon ``TextWriter``. The concept is implemented in similar way as ``Swappable`` concept with standalone function ``format()``::

   void ToFormat(TextWriter& out, const T& value, const StringSpan& opts);

This will print value with given options into the given writer.

These foundations enables implementation of ``TextWriter::format()``::

   void TextWriter::format(StringSpan fmt, const Ts&... args);

and provides an easy (C#/Python-like) way of formatting multiple objects.

See :ref:`stp-base-formattable` for details.

ToString
========

``std::to_string(T)`` simply converts a a type to ``std::string``.

``ToString(T, const StringSpan& opts)`` from Polonite is built upon ``Formattable`` concept and gives possibility for customization.

Once you define ``format()`` function for your type, you are able to use this type for: ``TextWriter::format()``, ``String::format()`` and ``ToString()``.

Signed vs Unsigned
==================

There are two different camps:

* use ``unsigned`` to denote numbers which should not become negative.
* use ``signed`` for quantities, ``unsigned`` for bitmasks only

Well, I am in second camp, period.

And for people who will argue with me (I've seen many arguments already) please note that my choice is not made by me alone. It is the opinion of C++ Standard Committee that made my choice. And they are more experienced than me, no doubt.

| **Bjarne Stroustrup in "The C++ Programming Language"**
|    "The ``unsigned`` integer types are ideal for uses that treat storage as a bit array. Using an ``unsigned`` instead of an ``int`` to gain one more bit to represent positive integers is almost never a good idea. Attempts to ensure that some values are positive by declaring variables ``unsigned`` will typically be defeated by the implicit conversion rules."

| **Scott Myers, Signed and Unsigned Types in Interfaces, 1995**

| **John Carmack on Twitter**
|    I've been trying to use ``size_t`` instead of ``int`` where appropriate, but the unsigned comparison errors are worse than the virtuous typing.

| `Pitfalls in C and C++: Unsigned types <http://www.soundsoftware.ac.uk/c-pitfall-unsigned>`_
| `CppCon 2016: Jon Kalb "unsigned: A Guideline for Better Code" <https://www.youtube.com/watch?v=wvtFGa6XJDU>`_
| `CppCon 2016: Chandler Carruth “Garbage In, Garbage Out: Arguing about Undefined Behavior..." <https://www.youtube.com/watch?v=yG1OZ69H_-o>`_
| `Interactive Panel: Ask Us Anything <https://youtu.be/Puio5dly9N8?t=2558>`_

For reasons stated above Polonite use ``signed`` numbers for quantities.

Iterators
=========

The iterator concept is not panacea to all problems.

The main purpose of some container types is not to iterate them. Consider hash map structure. Use of iterators in this case hides the real desire of this structure.

See HashMap/FlatMap interface to see the difference.

As the result, Iterators are used only for enumeration in Polonite.

Bytes
=====

C++17 introduced the distinct type ``std::byte``.

In Polonite we believe that arithmetic operations on bytes are useful in many scenarios.
And since ``std::byte`` is ``enum class`` these operations are hard to write.

The Polonite takes different approach and ``typedef``\s ``uint8_t`` to type ``byte_t``.

Moreover, there are many debates on ``std::string`` vs ``std::vector<char>``/``std::vector<std::byte>``.
Both classes have to wide interfaces for working with bytes.

Code Style
==========

At first you may think the code style is not important part.
But as you realize later, especially when mixing several mini projects, it may lead to a code that is  hard to read.

Polonite has very carefully thought :ref:`code style <stp-cpp-code-style>` to make it clean and readable.
