.. _stp-base-containers:

Containers
**********

Polonite provides its own set of container classes.

Common concepts
===============

Iterators
---------

The provided containers support iterator concept, but the usage is **NOT** the same though.
Non-contiguous containers (like ``HashMap<K, T>``) have concepts that are clear to express without iterators (and obscure with them).
As such iterators are only used for **enumeration** and nothing else.

Contiguous
========================

Contiguous containers just like ``T array[]`` are *ordered collections* with objects placed one after another in the memory.

``Array<T>`` is a fixed-size container, an alternative to ``T array[]`` with additional methods.

``List<T>`` on the other hand is a dynamically-sized array (std::vector replacement).

``Span<T>`` is a non-owning view to a part of contiguous sequence of objects. The mutability depends on constness of template argument.

.. note:: To work with bytes and strings other types are provided.

Hash Tables
===========

``HashMap<K, T>`` and ``HashSet<T>``

These are replacements for unordered associative containers in Standard C++ Library. Unlike ``std::unordered_map`` the ``HashMap`` does not use iterators for primary interface. Instead more natural functions are provided to operate.
