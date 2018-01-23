.. _stp-base-json:

Json
****

`RFC4627 <http://www.ietf.org/rfc/rfc4627.txt?number=4627>`_

.. _stp-base-json-value:

Working with Values
===================

A value in JSON can be either of following:

+--------+----------------+
| JSON   | Polonite       |
+========+================+
| null   |                |
+--------+                |
| boolean|                |
+--------+                |
| number | ``JsonValue``  |
+--------+                |
| string |                |
+--------+----------------+
| array  | ``JsonArray``  |
+--------+----------------+
| object | ``JsonObject`` |
+--------+----------------+

Several builtin types are supported by ``JsonValue``'s constructors::

   auto null_value = JsonValue(nullptr);
   auto boolean = JsonValue(true);
   auto integer = JsonValue(2);
   auto number = JsonValue(0.75);
   auto string = JsonValue("example");
   auto string2 = JsonValue(StringSpan("second example"));

``JsonValue`` differentiates between integer and double values. Some 64-bit integer values cannot be represented correctly by ``double`` type. Moreover, performance is better without converting numbers between float and integer types back and forth. The parser tries to represent the number with ``int64_t`` type first with fallback to ``double``.

To test what type has a value use one of ``Is*()`` methods::

   if (value.IsNumber())

To cast a value to one of builtin types use:

* unconditional ``As*()`` casts::

   JsonValue city = "New York";
   StringSpan city_str = city.AsString();
   // int32_t i = city.AsInteger(); // assertion

  Note that unconditional casts **asserts** if value is of requested type and performs no coercion.

* conditional ``TryCastTo*()`` which returns ``false``/``nullptr`` if value is not of given type::

   int32_t x;
   if (!value.TryCastTo(x))
     return false;

In above example ``TryCastTo()`` fails in two cases:

* ``value`` is not a number.
* ``value`` has a number outside of limits of target type (``int32_t``).

.. _stp-base-json-array:

Array
-----

``JsonArray`` class is an ordered sequence of JSON values. It has interface of ``List<JsonValue>``, but comes with several handy methods.

.. note:: The naming may be confusing here, since ``Array<T>`` is not resizable but ``JsonArray`` is. ``JsonList`` was considered, but since JSON specification mention array (not list) we use same convention for consistency.

To create a list of values use ``Add()`` or ``Set()``. Both of them will create a ``JsonValue`` on fly if given value is not of ``JsonValue`` type::

   #include "Base/Json/JsonArray.h"

   JsonArray CreateArray(const JsonValue& sixth_value) {
     JsonArray a;
     a.Add(3);
     a.Add("example string");

     a.Set(6, sixth_value);
     return a;
   }

The above code create an array of size ``7``. Values at ``2..5`` positions are null. Note that ``operator[]`` is different from ``Set()`` since the former will assert on access to non-existent index.

If you are not sure whether array holds an index, use ``TryGet()`` to obtain a value. There are several overloads which checks also for a type of the value and perform casts::

   JsonValue* opt_value = array.TryGet(3);

   JsonObject* object = array.TryGetObject(index);

   int number;
   if (!array.TryGet(10, number)
     return false;

.. _stp-base-json-object:

Object
------

``JsonObject`` class is an unordered container for a pairs of string key and JSON value. It has interface similar to ``FlatMap<String, JsonValue>``, but comes with several handy methods.

To fill an object with values use ``Add()``/``TryAdd()`` or ``Set()``. All of them will create a ``JsonValue`` on fly if given value is not of ``JsonValue`` type::

   #include "Base/Json/JsonObject.h"

   JsonObject CreateObject(const JsonValue& gender) {
     JsonObject o;
     // o["age"] = 20; // assertion - key must exist for operator[]
     o.Add("age", 30);
     o.Add("gender", gender);

     bool ok = o.TryAdd("gender", gender); // returns false - key already present
     // o.Add("gender", gender); // assertion
     o.Set("gender", "male"); // overwrites if exists

     return o;
   }

Key vs Path
^^^^^^^^^^^^^^^^

A handy set of ``*WithPath()`` helpers are provided to work with paths instead of keys. A path has the form ``<key>`` or ``<key>.<key>.[...]``, where ``.`` indexes into the next ``JsonObject`` down. Obviously, ``.`` can't be used within a key, but there are no other restrictions on keys.

For example ``SetWithPath()`` is ``Set()`` alternative which accepts path instead of key::

   object.Set("settings.global.full_screen", true);

If the key at any step of the way doesn't exist, or exists but isn't a ``JsonObject``, a new ``JsonObject`` will be created and attached to the path in that location.

``RemoveWithPath()`` removes a value at given path and has an option to control whether objects on path should be removed if they become empty.

.. _stp-base-json-parsing:

Parsing
=======

Each of value types has a method to deserialize a JSON string::

   JsonArray array;
   if (JsonArray::Parse(string, array, options)

In above example parsing may fail if input does not represent a JSON array, but other JSON type.

Use ``JsonParser`` class directly if you need error location and details.

The parsing is controlled with a set of :ref:`JsonOptions <stp-base-json-options>`.

Known limitations/deviations from the RFC:

* Only knows how to parse integers within the range of a signed 64 bit int and decimal numbers within a double.
* We limit nesting to 100 by default levels to prevent stack overflow (this is allowed by the RFC).

.. _stp-base-json-options:

Options
=======

Use ``JsonOptions`` to change parser/formatter behavior.

Each enumeration has its character equivalent (in parentheses) used in formatting and ``JsonOptions::Parse()``, e.g.::

   auto serialized = ToString(json_root, "C"); // allow commas

Common options:

* ``EnableInfNaN`` (``'N'``) - JSON specification has no notion of Infinity or Not-A-Number. This option introduces new values for double: Infinity, -Infinity and NaN.

**Parser**-only options:

* ``AllowTrailingCommas`` (``'C'``) -  JSON specification disallows a comma after last item in arrays and objects. This option weakens this rule.

* ``ReferenceInput`` (``''``) - The parser will perform an optimization by referencing input string in output tree. It means that JSON string values will point to the string given as input to the parser. This way the parser can skip many allocations and copies. **Be careful**: the input string given to the parser cannot go away (or use-after-free are likely). Analyze if JSON values are forwarded to external APIs. The referenced string (input for parser) must outlive all values.

* ``UniqueKeys`` - By default multiple values for the same key is allowed in dictionary while parsing. This option forces the opposite.

**Formatter**-only options:

* ``PrettyFormatting`` - Return a slightly nicer formatted JSON string (pads with whitespace to help with readability).

* ``EmitTrailingCommas`` - A comma is printed after last item in arrays and objects. This option has effect only when linked with ``PrettyFormatting``.

* ``EscapeUnicode`` - Non ASCII characters (UTF-8) are normally written as is. This option forces the formatter to escape unicode  using \uXXXX escape sequence. As a result of this option all characters in the output are ASCII.

* ``DisallowLossOfPrecision`` - Some of 64-bit integers cannot be represented in ``double``. Some parsers converts all numbers to ``double`` type. This option validates all integers to be correctly represented in ``double`` type.

* ``TryIntegerForFloat`` - This option instructs the formatter to write doubles that have no fractional part as a normal integer (i.e., without using exponential notation or appending a ``.0``) as long as the value is within the range of a 64-bit integer.

* ``BreakOnError`` - Normally formatter will continue on error (replaces ``NaN`` with zero for example). Pass this option to stop operation on first error.
