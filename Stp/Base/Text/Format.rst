.. _stp-base-formattable:

Formatting
**********

Polonite comes with its own formatting mechanism. It is easy to use and extend::

  auto msg_text = String::format(
      "Namespace of prefix '{}' not found, error: {:X}", prefix, 0x2345);

Each argument in format string is formattable with specification passed in braces. In above example ``:X`` specifies to format an integer in hexadecimal representation (upper-case).

Most classes in the library itself follows the formattable concept.
You can format single object with ``ToString()`` function like this::

  // optional argument specifies format options
  String str = ToString(bytes, "X");

In general you should stick to ``TextStream::format()`` though. Above examples are easy to show power, but efficiency comes with following code::

   void OutputJson(TextWriter& out, JsonObject& json) {
     out.format("JSON formatting with ease:\n {:PC}", json);
     out << "# end of file";
   }

To see more examples just scroll down to the :ref:`end of page <stp-base-formattable-more-examples>`.

Design Goals
============

* Efficient in terms of speed and memory.
* Extensibility with options passed to type-dependent formatters.
* Encoding support (any codec with UTF-8 as fast path).
* Locale-independent.
* Stateless (unlike I/O streams).
* Safety (with variadic arguments).
* Positional and named arguments

The implementation of formatting relies on ``TextStream``. Text streams gives inherent support for multiple encodings (on both input and output, without performance loss).

Use of ``TextStream`` enables to stream all text with **zero memory** allocation. Everything done on stack.

To make your class formattable just add following method::

  void ToFormat(TextWriter& writer, const StringSpan& opts) const;

or add function in the type's enclosing namespace::

  void format(TextWriter& writer, const SomeExternalType& value, const StringSpan& opts);

Additional ``options`` argument is a way to customize formatting of your class.
It is everything what comes after colon in format string.

.. _stp-base-formattable-more-examples:

Format String Syntax
====================

.. code-block:: bnf

   rep_field ::= "{" [arg_id] ["," layout] [":" options] "}"
   arg_id    ::= arg_index | arg_name
   arg_index ::= <non-negative integer>
   arg_name  ::= [_a-zA-Z][_a-zA-Z0-9]+ string
   layout    ::= [[[fill]align]width]
   options   ::= <any string not containing "{" or "}">
   fill      ::= <any character except "{" or "}">
   align     ::= "<" | "^" | ">"
   width     ::= <positive integer>

| ``rep_field``
| A single replacement fields enclosed by braces.

| ``arg_index``
| A non-negative integer specifying the index of the item in the parameter pack to print.

| ``arg_name``
| A string identifying one of the named items in parameter pack to print (when ``FMT_ARG()`` is used.

| ``layout``
| A string controlling how the field is laid out within the available space.

| ``options``
| A type-dependent string used to provide additional options to the formatting operation. Refer to the documentation of the various individual format providers for per-type options.

| ``fill``
| The padding character.
| Defaults to ``' '`` (space).
| Only valid if ``align`` is also specified.

| ``align``
| Where to print the formatted text within the field.
| Only valid if ``width`` is also specified.

* ``'<'`` : The field is left aligned within the available space.
* ``'^'`` : The field is centered within the available space.
* ``'>'`` : The field is right aligned within the available space (this is the default).

| ``width``
| The width of the field within which to print the formatted text.
| If this is less than the required length then the ``fill`` and ``align`` fields are ignored, and the field is printed with no leading or trailing padding. If this is greater than the required length, then the text is output according to the value of ``align``, and padded as appropriate on the left and/or right by ``fill``.

Special Characters
------------------

The characters ``'{'`` and ``'}'`` are reserved and cannot appear anywhere within a replacement sequence. Outside of a replacement sequence, in order to print a reserved character it must be doubled:

* ``"{{"`` to print left brace ``'{'``.
* ``"}}"`` to print right brace ``'}'``.

Fundamental Formatters
======================

In this section we present formatters for fundamental types.

Boolean Formatter
-----------------

The options string of a boolean type has the grammar:

.. code-block:: bnf

   bool_options ::= "" | "Y" | "y" | "D" | "d" | "T" | "t"

+----------+-------------------+
|  Option  |     Meaning       |
+==========+===================+
|     Y    | YES / NO          |
+----------+-------------------+
|     y    | yes / no          |
+----------+-------------------+
|   D / d  | Integer 0 or 1    |
+----------+-------------------+
|     T    | TRUE / FALSE      |
+----------+-------------------+
|     t    | true / false      |
+----------+-------------------+
|  (empty) | Equivalent to 't' |
+----------+-------------------+

Character Formatter
-------------------

The options string of a character type (``char``, ``char16_t``, etc.) has the grammar:

.. code-block:: bnf

   char_options  ::= "" | unicode | hex
   unicode       ::= ("U" | "u") ("4" | "8")
   hex           ::= ("X" | "x") ("4" | "8")

The meaning in following table represents how polish letter ``'Ś'`` is formatted.

+----------+-------------------+
| Option   |     Meaning       |
+==========+===================+
|  (empty) | Ś                 |
+----------+-------------------+
|     U4   | U+015A            |
+----------+-------------------+
|     u8   | U+0000015a        |
+----------+-------------------+
|     x4   | 015a              |
+----------+-------------------+
|     X8   | 0000015A          |
+----------+-------------------+

If ``U4`` is used and character is outside BMP, surrogate pair (``U+LEAD`` ``U+TRAIL``) is printed.

If ``X4`` is used, the character must be from BMP (assertion).

Integer Formatter
-----------------

The options string of an integral type has the grammar:

.. code-block:: bnf

   int_options ::= [sign] [format] [precision]
   sign        ::= "-" | "+"
   format      ::= "D" | "d' | "X" | "x" | "O" | "o"
   precision   ::= digit+
   digit       ::= 0..9

| ``sign``
| ``+`` forces either of signs to be printed
| ``-`` prefixes positive numbers with space

| ``format``
| Base system of printed value.
|   ``D``/``d`` - decimal
|   ``O``/``o`` - octal
|   ``X`` - hexadecimal upper-case
|   ``x`` - hexadecimal lower-case
| Defaults to ``'d'`` for signed, for unsigned defaults to ``'X'``.
| The reason unsigned values are printed with hexadecimal by default is they should be used for bit-masks only.

| ``precision``
| Minimum number of digits to be printed.
| If value to be printed has less digits, a number of zeros are written to fullfil the requirement.

Floating-point Formatter
------------------------

The options string of a floating-point type has the grammar:

.. code-block:: bnf

   float_options ::= [sign] [format] [precision]
   sign          ::= "-" | "+"
   format        ::= "G" | "g' | "F" | "f" | "E" | "e" | "P" | "p"
   precision     ::= digit+
   digit         ::= 0..9

| ``format``
| Defaults to ``"G"``.

| Default precision depends on ``format``:

* ``"G"`` - shortest of Fixed/Scientific forms is printed
* ``"F"`` - 5
* ``"E"`` - 5
* ``"P"`` - 2

.. todo:: Incomplete, explain each option in detail.

Pointer Formatter
-----------------

Any pointer is formatted as hexadecimal number with fixed-number of digits (depending on architecture) and prefixed with ``"0x"``.

If you want ``char*`` pointers to printed as strings use ``makeSpanFromNullTerminated()``.

More Examples
=============

.. code-block:: c++

   // Double '{' or '}' to print them.
   out.format("{{ {} }}", 5);
   // => "{5}"

   // Use positional arguments to reference single value multiple times.
   out.format("{0} + {0} != {}", 2, 5);
   // => "2 + 2 != 5"

   // Use named arguments to reference a value by name.
   out.format("{city} has {count}M people.", FMT_ARG(city), FMT_ARG(count));
   // => "New York has 9M people."

   // Layout your replacement:
   out.format("password = {}{*7}, 'p', 'd');
   // => "password = p******d"
